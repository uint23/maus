#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include "mauswin.h"

MausWindow* maus_init(const char* title, int x, int y, int width, int height)
{
	if (MAUS_WARN_BACKEND_AUTO_SEL)
		maus_log(stderr, "backend auto-selected: X11");

	Display* d = XOpenDisplay(NULL);
	if (!d)
		maus_die("could not open X11 display");

	uint32_t* px = malloc(sizeof(uint32_t)*width*height);

	MausWindow* win = malloc(sizeof(MausWindow));
	win->display = d;
	win->root = DefaultRootWindow(d);
	win->win = None;
	win->title = title;
	win->width = width;
	win->height = height;
	win->x = x;
	win->y = x;
	win->pixels = px;

	return win;
}

