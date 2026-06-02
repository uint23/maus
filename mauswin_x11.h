#ifndef MAUSWIN_X11_H
#define MAUSWIN_X11_H

#include <X11/Xlib.h>

typedef struct {
	Display*       display;
	Window         root;
	Window         win;

	const char*    title;
	int            width;
	int            height;
} MausWindow;

#endif /* MAUSWIN_X11_H */

