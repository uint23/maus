#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include "maus.h"

static bool handle_event(XEvent* xev, MausEvent* ev, Maus* mw);

static bool handle_event(XEvent* xev, MausEvent* ev, Maus* mw)
{
	switch (xev->type) {
		case ClientMessage:
			if ((Atom)xev->xclient.data.l[0] == mw->atoms[MAUS_ATOM_WM_DELETE_WINDOW]) {
				ev->type = MAUS_EV_CLOSE;
				return true;
			}
			break;

		case KeyPress:
			ev->type = MAUS_EV_KEY;
			ev->key.code = xev->xkey.keycode;
			ev->key.pressed = true;
			return true;

		case KeyRelease:
			ev->type = MAUS_EV_KEY;
			ev->key.code = xev->xkey.keycode;
			ev->key.pressed = false;
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

	Maus* win = malloc(sizeof(Maus));
	win->display = d;
	win->root = DefaultRootWindow(d);
	win->win = None;
	win->title = title;
	win->width = width;
	win->height = height;
	win->x = x;
	win->y = y;
	win->pixels = px;

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

