#define BACKEND_X11
#include "../../mauswin.h"

int main(void)
{
	MausWindow* mw = maus_init("basic window", 0, 0, 800, 600);
	maus_create_window(mw);

	XEvent e;
	for (;;)
		XNextEvent(mw->display, &e);

	maus_close(mw);
}

