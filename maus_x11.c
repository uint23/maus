#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include "maus.h"

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
	mw->win = XCreateSimpleWindow(
		mw->display, mw->root,
		mw->x, mw->y,
		mw->width, mw->height,
		0u, 0u, 0u
	);

	if (mw->win == None)
		return false;

	XMapWindow(mw->display, mw->win);
	XFlush(mw->display);

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

