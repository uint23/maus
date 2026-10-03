#ifndef MAUS_H
#define MAUS_H

#include <stdio.h>
#define ULE_SHORT_TYPE_NAMES
#include "uletypes.h"

#include "maus_input.h"

/* auto selection */
#if (!defined(BACKEND_WIN) && !defined(BACKEND_MAC) && \
    !defined(BACKEND_X11) && !defined(BACKEND_WAY)) || \
    defined(BACKEND_AUTO)

#ifdef BACKEND_AUTO
#define MAUS_WARN_BACKEND_AUTO_SEL 0
#else
#define MAUS_WARN_BACKEND_AUTO_SEL 1
#endif /* BACKEND_AUTO */

	#if defined(_WIN32)
	#define BACKEND_WIN

	#elif defined(__APPLE__)
	#define BACKEND_MAC

	#else /* default to X */
	#define BACKEND_X11

	#endif
#else
#define MAUS_WARN_BACKEND_AUTO_SEL 0
#endif /* auto selection */

#if defined(BACKEND_X11)
	#include "maus_x11.h"

#elif defined(BACKEND_WAY)
	#include "maus_wayland.h"

#elif defined(BACKEND_WIN)
	#include "maus_windows.h"

#elif defined(BACKEND_MAC)
	/* ... */
#endif

#define MAUS_KEYCODE_LAST (256)
#define MAUS_COL_ARGB(a, r, g, b) (MausColor){a, r, g, b}
#define MAUS_COL_RGBA(r, g, b, a) (MausColor){a, r, g, b}
#define MAUS_UNPACK_COL(c) \
	(((u32)(c).a << 24) | \
	 ((u32)(c).r << 16) | \
	 ((u32)(c).g <<  8) | \
	 ((u32)(c).b))

#define MAUS_PIXEL_AT(mw, x, y) ((mw)->bfb[(y) * (mw)->stride + (x)])

typedef enum {
	MAUS_EV_NONE,
	MAUS_EV_CLOSE,
	MAUS_EV_KEY,
	MAUS_EV_MOUSE_BUTTON,
	MAUS_EV_MOUSE_MOTION,
	MAUS_EV_RESIZE,
	MAUS_EV_REDRAW, /* TODO: MAUS_EV_REDRAW for windows */
	MAUS_EV_LAST
} MausEventType;

typedef struct {
	u8 a;
	u8 r;
	u8 g;
	u8 b;
} MausColor;

typedef struct {
	MausEventType type;

	struct {
		u32 code; /* raw, backend keycode */
		MausKey key;  /* logical, mapped key */
		char    text; /* translated text from key */
		i8      pressed;
	} key;

	struct {
		struct {
			MausMouseButton button;
			i8              pressed;
		} button;

		struct {
			i32 x;
			i32 y;
			i32 dx;
			i32 dy;
		} motion;
	} mouse;

	struct {
		u32 width;
		u32 height;
	} resize;
} MausEvent;

typedef struct {
	MausBackend backend;
	u64    frame_time_last;
	char*  clipboard;

	u32* fb;  /* front frame buffer */
	u32* bfb; /* back frame buffer */
	u32  stride;

	const char* title;
	u32 width;
	u32 height;
	i32 x;
	i32 y;

	i8 key_codes[MAUS_KEYCODE_LAST]; /* physical keys */
	i8 key_syms[MAUS_KEY_LAST];      /* logical keys */

	MausCursor cursor;
	i8         mouse_buttons[MAUS_MOUSE_BUTTON_LAST];
} Maus;

/* clear screen with color: `col` */
void maus_clear(Maus* mw, MausColor col);

/* set text to system clipboard */
void maus_clipboard_set_text(Maus* mw, const char* text);

/* get text from system clipboard */
char* maus_clipboard_get_text(Maus* mw);

/* close a Maus. returns 0 on fail
   TODO also free mw */
void maus_close(Maus* mw);

/* close a window without the whole Maus. returns 0 on fail */
i8 maus_close_window(Maus* mw);

/* create window from Maus. returns 0 on fail */
i8 maus_create_window(Maus* mw);

/* log message to output `fd` and die */
void maus_die(const char* fmt, ...);

/* get time since arbitrary start point (ns) */
u64 maus_get_time_ns(void);

/* initialise and fills the Maus. returns NULL on fail */
Maus* maus_init(const char* title, int x, int y, int width, int height);

/* log message to output `fd` */
void maus_log(FILE* fd, const char* fmt, ...);

/* poll for events then fill `ev` with retrieved events.
   returns 1 if event polled, else 0.
   note: can burn cpu cycles */
i8 maus_event_poll(Maus* mw, MausEvent* ev);

/* poll for events then fill `ev` with retrieved events. returns 1 if event
   polled, else 0. note, thread goes to sleep until an event arrives */
void maus_event_wait(Maus* mw, MausEvent* ev);

/* present the pixelbuffer to the screen */
void maus_present(Maus* mw);

/* resize window to specified width and height. returns 1 on resize success,
   else 0 */
i8 maus_resize(Maus* mw, u32 width, u32 height);

/* sleep for `ms` milliseconds */
void maus_sleep(u32 ms);

/* cap framerate to targetted fps */
void maus_target_fps(Maus* mw, u32 fps);

/* change behavior of mouse based on state passed */
void maus_cur_set_mode(Maus* mw, MausCursorState state);

#endif /* MAUS */

