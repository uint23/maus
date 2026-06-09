#ifndef MAUS_X11_H
#define MAUS_X11_H

#include <stdbool.h>
#include <stdint.h>

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

#include "maus_input.h"

#define MAUS_KEYCODE_LAST 256

typedef enum {
	MAUS_ATOM_WM_DELETE_WINDOW,
	MAUS_ATOM_LAST,
} MausX11Atoms;

typedef struct {
	Display*       display;
	Window         root;
	Window         win;
	Atom           atoms[MAUS_ATOM_LAST];
	GC             gc;

	XImage*        image;
	XShmSegmentInfo shm;
	bool           shmat;
	uint32_t*      fb;
	uint32_t       stride;

	const char*    title;
	uint32_t       width;
	uint32_t       height;
	int32_t        x;
	int32_t        y;

	bool           key_codes[MAUS_KEYCODE_LAST]; /* physical keys */
	bool           key_syms[MAUS_KEY_LAST];      /* logical keys */
	MausKey        keymap[MAUS_KEYCODE_LAST];    /* X11 keycode->MausKey */

	MausCursor     cursor;
} Maus;

#endif /* MAUS_X11_H */

