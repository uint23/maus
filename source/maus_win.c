#include <windows.h>

#include "maus.h"
#include "maus_win.h"

static bool fb_create(Maus* mw);
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

static bool fb_create(Maus* mw)
{
	MausBackend* be = &mw->backend;
	be->hbm = NULL;
	mw->fb = NULL;
	mw->stride = 0;

	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = mw->width;
	bmi.bmiHeader.biHeight = -mw->height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	HDC dc = GetDC(NULL);
	be->memdc = CreateCompatibleDC(dc);
	be->hbm = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, (void**)&mw->fb, NULL, 0);
	ReleaseDC(NULL, dc);

	if (!be->hbm || !mw->fb) {
		if (be->memdc)
			DeleteDC(be->memdc);
		maus_log(stderr, "CreateDIBSection failed");
		return false;
	}

	SelectObject(be->memdc, be->hbm);

	mw->stride = mw->width;
	return true;
}

static void fb_destroy(Maus* mw)
{
	MausBackend* be = &mw->backend;

	if (be->memdc) {
		DeleteDC(be->memdc);
		be->memdc = NULL;
	}
	if (be->hbm) {
		DeleteObject(be->hbm);
		be->hbm = NULL;
	}

	mw->fb = NULL;
	mw->stride = 0;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_DESTROY) {
		PostQuitMessage(EXIT_SUCCESS);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}

void maus_clear(Maus* mw, MausColor col)
{

}

void maus_clipboard_set_text(Maus* mw, const char* text)
{

}

char* maus_clipboard_get_text(Maus* mw)
{

}

void maus_close(Maus* mw)
{

}

bool maus_close_window(Maus* mw)
{

}


bool maus_create_window(Maus* mw)
{
	MausBackend* be = &mw->backend;
	be->hInst = GetModuleHandle(NULL);

	WNDCLASS wc = {0};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = be->hInst;
	wc.lpszClassName = "MausWindow";
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc))
		return false;

	RECT r = { 0, 0, mw->width, mw->height };
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);

	be->hwnd = CreateWindowEx(
		0, "MausWindow", mw->title,
		WS_OVERLAPPEDWINDOW, mw->x, mw->y,
		r.right - r.left, r.bottom - r.top,
		NULL, NULL, be->hInst, NULL
	);

	if (!be->hwnd)
		return false;

	SetWindowLongPtr(be->hwnd, GWLP_USERDATA, (LONG_PTR)mw);
	ShowWindow(be->hwnd, SW_SHOW);
	UpdateWindow(be->hwnd);
	return true;
}

Maus* maus_init(const char* title, int x, int y, int width, int height)
{
	Maus* mw = calloc(1, sizeof(Maus));
	MausBackend* be = &mw->backend;

	mw->frame_time_last = maus_get_time_ns();
	mw->title = title;
	mw->width = width;
	mw->height = height;
	mw->x = x;
	mw->y = y;
	be->hInst = GetModuleHandle(NULL);
	be->hwnd = NULL;
	be->hbm = NULL;

	if (!fb_create(mw)) {
		maus_log(stderr, "failed to create framebuffer");
		free(mw);
		return NULL;
	}

	mw->bfb = calloc(mw->stride * mw->height, sizeof(uint32_t));
	if (!mw->bfb) {
		maus_log(stderr, "failed to allocate back buffer");
		maus_close(mw);
		free(mw);
		return NULL;
	}

	return mw;
}

bool maus_event_poll(Maus* mw, MausEvent* ev)
{

}

void maus_event_wait(Maus* mw, MausEvent* ev)
{

}

void maus_present(Maus* mw)
{

}

bool maus_resize(Maus* mw, uint32_t width, uint32_t height)
{

}

void maus_cur_set_mode(Maus* mw, MausCursorState state)
{

}

