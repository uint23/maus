#ifndef MAUS_X11_H
#define MAUS_X11_H

#include <stdbool.h>
#include <stdint.h>

#include <X11/Xlib.h>

typedef enum {
	MAUS_ATOM_WM_DELETE_WINDOW,
	MAUS_ATOM_LAST,
} MausX11Atoms;

typedef struct {
	Display*       display;
	Window         root;
	Window         win;
	Atom           atoms[MAUS_ATOM_LAST];

	const char*    title;
	uint32_t       width;
	uint32_t       height;
	int32_t        x;
	int32_t        y;

	uint32_t*      pixels;
	bool           keys[256];
} Maus;

#endif /* MAUS_X11_H */

