#include <stdbool.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/extensions/XShm.h>

#include "maus.h"
#include "maus_input.h"

typedef struct {
	KeySym  x11;
	MausKey maus;
} KeyMapEntry;

static const KeyMapEntry keymap[] = {
	{ XK_BackSpace,    MAUS_KEY_BACKSPACE }, { XK_Tab,              MAUS_KEY_TAB },
	{ XK_Return,       MAUS_KEY_ENTER },     { XK_Escape,           MAUS_KEY_ESCAPE },
	{ XK_apostrophe,   MAUS_KEY_APOSTROPHE },{ XK_comma,            MAUS_KEY_COMMA },
	{ XK_space,        MAUS_KEY_SPACE },     { XK_minus,            MAUS_KEY_MINUS },
	{ XK_period,       MAUS_KEY_PERIOD },    { XK_slash,            MAUS_KEY_SLASH },
	{ XK_semicolon,    MAUS_KEY_SEMICOLON }, { XK_equal,            MAUS_KEY_EQUAL },
	{ XK_Delete,       MAUS_KEY_DELETE },    { XK_Left,             MAUS_KEY_LEFT },
	{ XK_Right,        MAUS_KEY_RIGHT },     { XK_Up,               MAUS_KEY_UP },
	{ XK_Down,         MAUS_KEY_DOWN },      { XK_Home,             MAUS_KEY_HOME },
	{ XK_End,          MAUS_KEY_END },       { XK_Page_Up,          MAUS_KEY_PAGE_UP },
	{ XK_Page_Down,    MAUS_KEY_PAGE_DOWN }, { XK_Insert,           MAUS_KEY_INSERT },
	{ XK_Shift_L,      MAUS_KEY_SHIFT_L },   { XK_Shift_R,          MAUS_KEY_SHIFT_R },
	{ XK_Control_L,    MAUS_KEY_CONTROL_L }, { XK_Control_R,        MAUS_KEY_CONTROL_R },
	{ XK_Alt_L,        MAUS_KEY_ALT_L },     { XK_Alt_R,            MAUS_KEY_ALT_R },
	{ XK_Super_L,      MAUS_KEY_SUPER_L },   { XK_Super_R,          MAUS_KEY_SUPER_R },
	{ XK_Caps_Lock,    MAUS_KEY_CAPS_LOCK }, { XK_Num_Lock,         MAUS_KEY_NUM_LOCK },
	{ XK_Pause,        MAUS_KEY_PAUSE },     { XK_Menu,             MAUS_KEY_MENU },
	{ XK_KP_Add,       MAUS_KEY_KP_ADD },    { XK_KP_Enter,         MAUS_KEY_KP_ENTER },
	{ XK_bracketright, MAUS_KEY_RIGHT_BRACKET }, { XK_grave,        MAUS_KEY_GRAVE },
	{ XK_Scroll_Lock,  MAUS_KEY_SCROLL_LOCK },   { XK_Print,        MAUS_KEY_PRINT_SCREEN },
	{ XK_KP_Decimal,   MAUS_KEY_KP_DECIMAL },    { XK_KP_Divide,    MAUS_KEY_KP_DIVIDE },
	{ XK_KP_Multiply,  MAUS_KEY_KP_MULTIPLY },   { XK_KP_Subtract,  MAUS_KEY_KP_SUBTRACT },
	{ XK_bracketleft,  MAUS_KEY_LEFT_BRACKET },  { XK_backslash,    MAUS_KEY_BACKSLASH },
	{ XK_KP_Equal,     MAUS_KEY_KP_EQUAL },
};

static int xerrored;

static void build_keymap(Maus* mw);
static bool fb_create(Maus* mw);
static bool fb_create_shm(Maus* mw);
/* TODO? static bool fb_create_ximage(Maus* mw); */
static void fb_destroy(Maus* mw);
static bool handle_event(XEvent* xev, MausEvent* ev, Maus* mw);
static MausKey keysym_to_mauskey(KeySym sym);
static MausMouseButton mouse_button_to_maus(int btn);
static int xerr(Display* dpy, XErrorEvent* ev);

static void build_keymap(Maus* mw)
{
	int min_code = 0;
	int max_code = 0;
	int syms_per_code = 0;

	/* can be called multiple times during lieftime
	   so initial calloc may not be enough */
	for (int i = 0; i < MAUS_KEYCODE_LAST; i++)
		mw->keymap[i] = MAUS_KEY_NONE;

	XDisplayKeycodes(mw->display, &min_code, &max_code);
	KeySym* syms = XGetKeyboardMapping(
		mw->display, min_code,
		max_code - min_code + 1, &syms_per_code
	);
	if (!syms)
		return;
	if (syms_per_code <= 0) {
		XFree(syms);
		return;
	}

	for (int code = min_code; code <= max_code && code < MAUS_KEYCODE_LAST; code++) {
		for (int col = 0; col < syms_per_code; col++) {
			KeySym sym = syms[(code - min_code) * syms_per_code + col];
			mw->keymap[code] = keysym_to_mauskey(sym);
			if (mw->keymap[code] != MAUS_KEY_NONE)
				break;
		}
	}

	XFree(syms);
}

static bool fb_create(Maus* mw)
{
	mw->image = NULL;
	mw->fb = NULL;
	mw->stride = 0;
	mw->shmat = false;
	mw->shm.shmid = -1;
	mw->shm.shmaddr = NULL;

	return fb_create_shm(mw);
}

/* allocate and store a new shm for the framebuffer */
static bool fb_create_shm(Maus* mw)
{

	if (!XShmQueryExtension(mw->display))
		return false;

	int scr = DefaultScreen(mw->display);
	int depth = DefaultDepth(mw->display, scr);
	Visual* vis = DefaultVisual(mw->display, scr);
	if (!vis) {
		maus_log(stderr, "could not get default visual");
		return false;
	}

	mw->image = XShmCreateImage(
		mw->display, vis, depth, ZPixmap, NULL,
		&mw->shm, mw->width, mw->height
	);
	if (!mw->image) {
		maus_log(stderr, "could not create XImage (shm)");
		return false;
	}

	if (mw->image->bits_per_pixel != 32) {
		XDestroyImage(mw->image);
		mw->image = NULL;
		maus_log(stderr, "XImage not 32bpp");
		return false;
	}

	size_t size = mw->image->bytes_per_line * mw->image->height;
	mw->shm.shmid = shmget(IPC_PRIVATE, size, IPC_CREAT | 0600);
	if (mw->shm.shmid < 0) {
		XDestroyImage(mw->image);
		mw->image = NULL;
		maus_log(stderr, "shmget() failed");
		return false;
	}

	mw->shm.shmaddr = shmat(mw->shm.shmid, NULL, 0);
	if (mw->shm.shmaddr == (char*)-1) {
		shmctl(mw->shm.shmid, IPC_RMID, NULL);
		XDestroyImage(mw->image);
		mw->image = NULL;
		maus_log(stderr, "shmat() failed");
		return false;
	}

	mw->shm.readOnly = False;
	mw->image->data = mw->shm.shmaddr;

	xerrored = 0;
	int (*old_xerr)(Display*, XErrorEvent*) = XSetErrorHandler(xerr);

	if (!XShmAttach(mw->display, &mw->shm))
		xerrored = 1;

	XSync(mw->display, False);
	XSetErrorHandler(old_xerr);
	if (xerrored) {
		shmdt(mw->shm.shmaddr);
		shmctl(mw->shm.shmid, IPC_RMID, NULL);
		XDestroyImage(mw->image);

		mw->image = NULL;
		mw->fb = NULL;
		mw->shm.shmaddr = NULL;
		mw->shm.shmid = -1;
		maus_log(stderr, "XShmAttach failed");
		return false;
	}

	shmctl(mw->shm.shmid, IPC_RMID, NULL);

	mw->shmat = true;
	mw->fb = (uint32_t*) mw->image->data;
	mw->stride = mw->image->bytes_per_line / sizeof(uint32_t);

	return true;
}

static void fb_destroy(Maus* mw)
{
	Display* dpy = mw->display;
	if (mw->shmat) {
		XShmDetach(dpy, &mw->shm);
		XSync(dpy, False);
		mw->shmat = false;
	}

	if (mw->image) {
		XDestroyImage(mw->image);
		mw->image = NULL;
	}

	if (mw->shm.shmaddr && mw->shm.shmaddr != (char*) -1) {
		shmdt(mw->shm.shmaddr);
		mw->shm.shmaddr = NULL;
	}

	/* normally already removed after attach but
	   can check anyways */
	if (mw->shm.shmid >= 0) {
		shmctl(mw->shm.shmid, IPC_RMID, NULL);
		mw->shm.shmid = -1;
	}

	mw->fb = NULL;
	mw->stride = 0;
}

static bool handle_event(XEvent* xev, MausEvent* ev, Maus* mw)
{
	uint32_t code = 0;
	unsigned int mb;
	MausMouseButton mbtype;
	MausKey key = MAUS_KEY_NONE;

	switch (xev->type) {
		case ClientMessage:
			if ((Atom)xev->xclient.data.l[0] == mw->atoms[MAUS_ATOM_WM_DELETE_WINDOW]) {
				ev->type = MAUS_EV_CLOSE;
				return true;
			}
			break;

		case MappingNotify:
			XRefreshKeyboardMapping(&xev->xmapping);
			build_keymap(mw);
			break;

		case KeyPress:
			ev->type = MAUS_EV_KEY;
			code = xev->xkey.keycode;
			key = code < MAUS_KEYCODE_LAST ? mw->keymap[code] : MAUS_KEY_NONE;
			ev->key.code = code;
			ev->key.key = key;
			ev->key.pressed = true;
			if (code < MAUS_KEYCODE_LAST)
				mw->key_codes[code] = true;
			if (key != MAUS_KEY_NONE)
				mw->key_syms[key] = true;
			return true;

		case KeyRelease:
			ev->type = MAUS_EV_KEY;
			code = xev->xkey.keycode;
			key = code < MAUS_KEYCODE_LAST ? mw->keymap[code] : MAUS_KEY_NONE;
			ev->key.code = code;
			ev->key.key = key;
			ev->key.pressed = false;
			if (code < MAUS_KEYCODE_LAST)
				mw->key_codes[code] = false;
			if (key != MAUS_KEY_NONE)
				mw->key_syms[key] = false;
			return true;

		case ButtonPress:
			ev->type = MAUS_EV_MOUSE_BUTTON;
			mb = xev->xbutton.button;
			mbtype = mouse_button_to_maus(mb);
			ev->mouse.button.button = mbtype;
			ev->mouse.button.pressed = true;
			mw->mouse_buttons[mbtype] = true;
			return true;

		case ButtonRelease:
			ev->type = MAUS_EV_MOUSE_BUTTON;
			mb = xev->xbutton.button;
			mbtype = mouse_button_to_maus(mb);
			ev->mouse.button.button = mbtype;
			ev->mouse.button.pressed = false;
			mw->mouse_buttons[mbtype] = false;
			return true;

		case MotionNotify:
			ev->type = MAUS_EV_MOUSE_MOTION;
			ev->mouse.motion.x = xev->xmotion.x;
			ev->mouse.motion.y = xev->xmotion.y;
			return true;

		case ConfigureNotify:
			ev->type = MAUS_EV_RESIZE;
			ev->resize.width = xev->xconfigure.width;
			ev->resize.height = xev->xconfigure.height;
			return true;
	}

	return false;
}

static MausKey keysym_to_mauskey(KeySym sym)
{
	if (sym >= XK_0 && sym <= XK_9)
		return MAUS_KEY_0 + (sym - XK_0);
	if (sym >= XK_A && sym <= XK_Z)
		return MAUS_KEY_A + (sym - XK_A);
	if (sym >= XK_a && sym <= XK_z)
		return MAUS_KEY_A + (sym - XK_a);
	if (sym >= XK_F1 && sym <= XK_F12)
		return MAUS_KEY_F1 + (sym - XK_F1);
	if (sym >= XK_KP_0 && sym <= XK_KP_9)
		return MAUS_KEY_KP_0 + (sym - XK_KP_0);

	for (size_t i = 0; i < sizeof(keymap)/ sizeof(keymap[0]); i++) {
		if (keymap[i].x11 == sym)
			return keymap[i].maus;
	}

	return MAUS_KEY_NONE;
}


void maus_close(Maus* mw)
{
	fb_destroy(mw);
	if (mw->win != None) {
		(void) maus_close_window(mw);
	}
	if (mw->display)
		XCloseDisplay(mw->display);
}

bool maus_close_window(Maus* mw)
{
	if (mw->gc) {
		XFreeGC(mw->display, mw->gc);
		mw->gc = 0;
	}

	if (mw->win != None) {
		XUnmapWindow(mw->display, mw->win);
		XDestroyWindow(mw->display, mw->win);
	}

	return true;
}

bool maus_create_window(Maus* mw)
{
	Display* dpy = mw->display;
	Window* win = &mw->win;

	*win = XCreateSimpleWindow(
		dpy, mw->root,
		mw->x, mw->y,
		mw->width, mw->height,
		0u, 0u, 0u
	);

	if (*win == None)
		return false;

	XSelectInput(dpy, *win,
		ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask |
		ButtonReleaseMask | PointerMotionMask | StructureNotifyMask
	);

	/* atoms */
	mw->atoms[MAUS_ATOM_WM_DELETE_WINDOW] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);

	XSetWMProtocols(dpy, *win, &mw->atoms[MAUS_ATOM_WM_DELETE_WINDOW], 1);
	XStoreName(dpy, *win, mw->title);

	XMapWindow(dpy, *win);
	XFlush(dpy);

	mw->gc = XCreateGC(dpy, *win, 0, NULL);
	if (!mw->gc) {
		maus_log(stderr, "failed to create window GC");
		maus_close_window(mw);
		return false;
	}

	return true;
}

void maus_fb_clear(Maus* mw, MausColor col)
{
	for (uint32_t y = 0; y < mw->height; y++) {
		uint32_t* row = mw->fb + (y * mw->stride);
		for (uint32_t x = 0; x < mw->width; x++)
			row[x] = (uint32_t) MAUS_UNPACK_COL(col);
	}
}

Maus* maus_init(const char* title, int x, int y, int width, int height)
{
	if (MAUS_WARN_BACKEND_AUTO_SEL)
		maus_log(stderr, "backend auto-selected: X11");

	Display* d = XOpenDisplay(NULL);
	if (!d) {
		maus_die("failed to open X11 display");
		return NULL;
	}

	Maus* mw = calloc(1, sizeof(Maus));
	mw->display = d;
	mw->root = DefaultRootWindow(d);
	mw->win = None;
	mw->title = title;
	mw->width = width;
	mw->height = height;
	mw->x = x;
	mw->y = y;
	mw->fb = NULL;

	build_keymap(mw);
	
	mw->gc = 0;
	mw->image = NULL;
	mw->fb = NULL;
	mw->shmat = false;
	mw->shm.shmid = -1;
	mw->shm.shmaddr = NULL;
	if (!fb_create(mw)) {
		maus_log(stderr, "failed to create framebuffer");
		maus_close(mw);
		free(mw);
		return NULL;
	}

	return mw;
}

bool maus_event_poll(Maus* mw, MausEvent* ev)
{
	XEvent xev;
	while (XPending(mw->display)) {
		XNextEvent(mw->display, &xev);
		ev->type = MAUS_EV_NONE;
		if (handle_event(&xev, ev, mw))
			return true;

	}

	return false;
}

static MausMouseButton mouse_button_to_maus(int btn)
{
	switch (btn) {
		case Button1: return MAUS_MOUSE_BUTTON_LEFT;
		case Button2: return MAUS_MOUSE_BUTTON_MIDDLE;
		case Button3: return MAUS_MOUSE_BUTTON_RIGHT;
		case Button4: return MAUS_MOUSE_BUTTON_SCROLL_UP;
		case Button5: return MAUS_MOUSE_BUTTON_SCROLL_DOWN;
		default:      return MAUS_MOUSE_BUTTON_NONE;
	}
}

static int xerr(Display* dpy, XErrorEvent* ev)
{
	(void) dpy;
	(void) ev;
	xerrored = 1;
	return 0;
}

void maus_event_wait(Maus* mw, MausEvent* ev)
{
	XEvent xev;
	for (;;) {
		XNextEvent(mw->display, &xev);
		ev->type = MAUS_EV_NONE;
		if (handle_event(&xev, ev, mw))
			return;
	}
}

void maus_present(Maus* mw)
{
	if (!mw->image || mw->win == None)
		return;

	XShmPutImage(
		mw->display, mw->win, mw->gc, mw->image,
		0, 0, 0, 0, mw->width, mw->height, False
	);

	XFlush(mw->display);
}

bool maus_resize(Maus* mw, uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0 ||
	    mw->width == width || mw->height == height)
		return false;
	fb_destroy(mw);

	mw->width = width;
	mw->height = height;

	XSync(mw->display, False);
	XFlush(mw->display);
	return fb_create(mw);
}

void maus_cur_set_mode(Maus* mw, MausCursorState state)
{
	Display* dpy = mw->display;
	Window win = mw->win;

	if (!dpy) {
		maus_log(stderr, "display is NULL");
		return;
	}
	if (win == None) {
		maus_log(stderr, "win is None");
		return;
	}

	/* show cursor */
	if (state == MAUS_CURSOR_STATE_VISIBLE) {
		XUndefineCursor(dpy, win);
		XFlush(dpy);
		return;
	}

	/* hide cursor */
	if (state == MAUS_CURSOR_STATE_HIDDEN) {
		Pixmap blank_pm = XCreatePixmap(dpy, win, 1, 1, 1);
		XColor black = {.red=0, .green=0, .blue=0};
		Cursor blank_cur = XCreatePixmapCursor(
			dpy, blank_pm, blank_pm, &black, &black, 0, 0
		);
		XFreePixmap(dpy, blank_pm);

		XDefineCursor(dpy, win, blank_cur);
		XFreeCursor(dpy, blank_cur);
		XFlush(dpy);
		return;
	}

	/* lock cursor */
	if (state == MAUS_CURSOR_STATE_LOCKED) {
		Mask mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
		int grab = XGrabPointer(
			dpy, win, True, mask, GrabModeAsync,
			GrabModeAsync, win, None, CurrentTime
		);

		if (grab != GrabSuccess) {
			maus_log(stderr, "failed to grab mouse pointer");
			return;
		}

		/* TODO keep position just snap to window dimensions */
		XWarpPointer(dpy, None, win, 0, 0, 0, 0, mw->width/2, mw->height/2);
		XFlush(dpy);
		return;
	}

	/* unlock cursor */
	if (state == MAUS_CURSOR_STATE_FREE) {
		XUngrabPointer(dpy, CurrentTime);
		XFlush(dpy);
		return;
	}
}

