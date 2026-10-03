#ifndef MAUS_X11_H
#define MAUS_X11_H

#define ULE_SHORT_TYPE_NAMES
#include "uletypes.h"

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

typedef enum {
	MAUS_ATOM_WM_DELETE_WINDOW,
	MAUS_ATOM_LAST
} MausX11Atoms;

typedef struct {
	Display* display;
	Window   root;
	Window   win;
	Atom     atoms[MAUS_ATOM_LAST];
	GC       gc;

	i8 cur_rel; /* relative cursor */
	i8 ignore_warp;
	i8 xi2;
	int    xi2_opcode;
	i32 mouse_x;
	i32 mouse_y;
	i8  mouse_pos_set;

	XShmSegmentInfo shm;
	i8          shmat;
	XImage*         image; /* TODO: XImage path */
} MausBackend;

#endif /* MAUS_X11_H */

