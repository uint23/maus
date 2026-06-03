#ifndef MAUSWIN_X11_H
#define MAUSWIN_X11_H

#include <stdbool.h>
#include <stdint.h>

#include <X11/Xlib.h>

typedef struct {
	Display*       display;
	Window         root;
	Window         win;

	const char*    title;
	uint32_t       width;
	uint32_t       height;
	int32_t        x;
	int32_t        y;

	uint32_t*      pixels;
} Maus;

#endif /* MAUSWIN_X11_H */

