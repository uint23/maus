#include <stdlib.h>

#include "../../maus.h"

void handle_ev(MausEvent* ev, Maus* mw)
{
	switch (ev->type) {
		case MAUS_EV_CLOSE:
			maus_close(mw);
			exit(EXIT_SUCCESS);
		case MAUS_EV_KEY:
			if (ev->key.code == 24) { /* quit */
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
	maus_create_window(mw);

	MausEvent ev;
	for (;;) {
		while (maus_poll(mw, &ev))
			handle_ev(&ev, mw);
	}

	maus_close(mw);
}

