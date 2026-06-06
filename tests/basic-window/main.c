#include <stdlib.h>

#include "../../maus.h"

int mx, my;

void handle_ev(Maus* mw, MausEvent* ev)
{
	switch (ev->type) {
		case MAUS_EV_CLOSE:
			maus_close(mw);
			exit(EXIT_SUCCESS);
			break;
		case MAUS_EV_KEY:
			if (ev->key.key == MAUS_KEY_Q) {
				maus_close(mw);
				exit(EXIT_SUCCESS);
			} break;
		case MAUS_EV_MOUSE_BUTTON:
			break;
		case MAUS_EV_MOUSE_MOTION: {
			mx = ev->mouse.motion.x;
			my = ev->mouse.motion.y;

		} break;
		case MAUS_EV_RESIZE: {
			maus_resize(mw, ev->resize.width, ev->resize.height);
		} break;
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
	// waiting -- used in applications
	/* for (;;) { */
	/* 	maus_event_wait(mw, &ev); */
	/* 	handle_ev(&ev, mw); */
	/* } */


	// constant polling -- used in games
	maus_fb_clear(mw, MAUS_COL_RGBA(255, 255, 255, 255));
	MausColor red = { 255, 255, 0, 0 };
	for (;;) {
		while (maus_event_poll(mw, &ev))
			(void) handle_ev(mw, &ev);
		if (mx >= 0 && my >= 0 &&
		    mx < (int32_t)mw->width &&
		    my < (int32_t)mw->height) {
			MAUS_PIXEL_AT(mw, mx, my) = MAUS_UNPACK_COL(red);
		}

		red.b++;
		for (uint32_t y = 0; y < mw->height/2; y++) {
			for (uint32_t x = 0; x < mw->width/2; x++) {
				MAUS_PIXEL_AT(mw, x, y) = MAUS_UNPACK_COL(red);
			}
		}

		maus_present(mw);
	}

	// rainbow
	/* uint8_t r = 0; */
	/* uint8_t g = 86; */
	/* uint8_t b = 172; */
	/* for (;;) { */
	/* 	while (maus_event_poll(mw, &ev)) */
	/* 		(void) handle_ev(mw, &ev); */

	/* 	maus_fb_clear(mw, MAUS_COL_RGBA(r++, g++, b++, 255)); */
	/* 	maus_present(mw); */
	/* } */

	maus_close(mw);
}

