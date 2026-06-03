#ifndef MAUSWIN_H
#define MAUSWIN_H

#include <stdbool.h>
#include <stdio.h>

/* auto selection */
#if !defined(BACKEND_WIN) || !defined(BACKEND_MAC) || \
    !defined(BACKEND_X11) || !defined(BACKEND_WAY)

#define MAUS_WARN_BACKEND_AUTO_SEL 1

#if defined(_WIN32)
#define BACKEND_WIN32

#elif defined(__APPLE__)
#define BACKEND_COCOA

#else /* default to X not Way */
#define BACKEND_X11

#endif
#endif /* auto selection */

#if defined(BACKEND_X11)
#include "maus_x11.h"

#elif defined(BACKEND_WAY)
/* ... */

#elif defined(BACKEND_WIN)
/* ... */

#elif defined(BACKEND_MAC)
/* ... */

#endif

typedef enum {
	MAUS_EV_NONE,
	MAUS_EV_CLOSE,
	MAUS_EV_KEY,
	MAUS_EV_MOUSE_BUTTON,
	MAUS_EV_MOUSE_MOTION,
	MAUS_EV_RESIZE,
} MausEventType;

typedef struct {
	MausEventType type;

	union {
		struct {
			uint32_t  code;
			/*  sym */
			bool      pressed;
		} key;

		struct {
			struct {
				uint8_t  button;
				bool     pressed;
			} button;

			struct {
				int32_t  x;
				int32_t  y;
			} motion;
		} mouse;

		struct {
			uint32_t width;
			uint32_t height;
		} resize;
	};
} MausEvent;

/* close a Maus. returns false on fail */
bool maus_close(Maus* mw);

/* close a window without the whole Maus. returns false on fail */
bool maus_close_window(Maus* mw);

/* create window from Maus. returns false on fail */
bool maus_create_window(Maus* mw);

/* log message to output `fd` and die */
void maus_die(const char* fmt, ...);

/* initialise and fills the Maus. returns NULL on fail */
Maus* maus_init(const char* title, int x, int y, int width, int height);

/* log message to output `fd` */
void maus_log(FILE* fd, const char* fmt, ...);

/* poll for events then fill `ev` with retrieved events */
bool maus_poll(Maus* mw, MausEvent* ev);

#endif /* MAUSWIN_H */

