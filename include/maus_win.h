#ifndef MAUS_WIN_H
#define MAUS_WIN_H

#include <stdbool.h>

#include <windows.h>

typedef struct {
	HWND           hwnd;
	HINSTANCE      hInst;
	HDC            hdc;
	HBITMAP        hbm;
	HDC            memdc;
} MausBackend;

#endif /* MAUS_WIN_H */

