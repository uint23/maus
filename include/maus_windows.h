#ifndef MAUS_WINDOWS_H
#define MAUS_WINDOWS_H

#include "uletypes.h"

#include <windows.h>

typedef struct {
	HWND      hwnd;
	HINSTANCE hInst;
	HDC       hdc;
	HBITMAP   hbm;
	HDC       memdc;
	ule_i8    resized;
	ule_u32  rw;
	ule_u32  rh;
} MausBackend;

#endif /* MAUS_WINDOWS_H */

