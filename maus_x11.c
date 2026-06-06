#include <stdbool.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "maus.h"

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

static void build_keymap(Maus* mw);
static bool handle_event(XEvent* xev, MausEvent* ev, Maus* mw);
static MausKey keysym_to_mauskey(KeySym sym);

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

static bool handle_event(XEvent* xev, MausEvent* ev, Maus* mw)
{
	uint32_t code = 0;
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
			ev->mouse.button.button = xev->xbutton.button;
			ev->mouse.button.pressed = true;
			return true;

		case ButtonRelease:
			ev->type = MAUS_EV_MOUSE_BUTTON;
			ev->mouse.button.button = xev->xbutton.button;
			ev->mouse.button.pressed = false;
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

			/* TODO put into maus_resize */
			mw->width = xev->xconfigure.width;
			mw->height = xev->xconfigure.height;
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


bool maus_close(Maus* mw)
{
	if (mw->pixels)
		free(mw->pixels);
	if (mw->win != None) {
		(void) maus_close_window(mw);
	}
	if (mw->display)
		XCloseDisplay(mw->display);

	return true;
}

bool maus_close_window(Maus* mw)
{
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

	return true;
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

	uint32_t* px = malloc(sizeof(uint32_t)*width*height);
	if (!px) {
		maus_log(stderr, "failed to allocate %dx%d pixel grid", width, height);
		return NULL;
	}

	Maus* win = calloc(1, sizeof(Maus));
	win->display = d;
	win->root = DefaultRootWindow(d);
	win->win = None;
	win->title = title;
	win->width = width;
	win->height = height;
	win->x = x;
	win->y = y;
	win->pixels = px;

	build_keymap(win);

	return win;
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

