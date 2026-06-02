#ifndef MAUSWIN_H
#define MAUSWIN_H

#include <stdio.h>

/* auto selection */
#if !defined(BACKEND_WIN) && !defined(BACKEND_MAC) && \
    !defined(BACKEND_X11) && !defined(BACKEND_WAY)

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
#include "mauswin_x11.h"

#elif defined(BACKEND_WAY)
/* ... */

#elif defined(BACKEND_WIN)
/* ... */

#elif defined(BACKEND_MAC)
/* ... */

#endif

void maus_die(const char* fmt, ...);
MausWindow* maus_init(const char* title, int width, int height);
void maus_log(FILE* fd, const char* fmt, ...);

#endif /* MAUSWIN_H */

