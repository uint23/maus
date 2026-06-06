#include <stdlib.h>

#include "../../maus.h"

void handle_ev(MausEvent* ev, Maus* mw)
{
	switch (ev->type) {
		case MAUS_EV_CLOSE:
			maus_close(mw);
			exit(EXIT_SUCCESS);
		case MAUS_EV_KEY:
			if (ev->key.key == MAUS_KEY_Q) {
				maus_close(mw);
				exit(EXIT_SUCCESS);
			}
		case MAUS_EV_MOUSE_BUTTON:
			break;
		case MAUS_EV_MOUSE_MOTION:
			break;
		case MAUS_EV_RESIZE:
			break;
		case MAUS_EV_NONE:
			break;
	}
}

int main(void)
{
	Maus* mw = maus_init("basic window", 0, 0, 800, 600);
	if (!mw)
		return EXIT_FAILURE;
	maus_create_window(mw);

	MausEvent ev;
	/* waiting -- used in applications
	for (;;) {
		maus_event_wait(mw, &ev);
		handle_ev(&ev, mw);
	}
	*/

	/* constant polling -- used in games */
	for (;;) {
		while (maus_event_poll(mw, &ev))
			(void) handle_ev(&ev, mw);
		maus_fb_clear(mw, MAUS_COL_RGBA(255, 255, 255, 255));
		maus_present(mw);
	}

	maus_close(mw);
}

