#if !defined(BACKEND_WIN)
#define _POSIX_C_SOURCE 199309L
#else
#include <windows.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "maus.h"

static void vlog(FILE* fd, const char* fmt, va_list ap);

static void vlog(FILE* fd, const char* fmt, va_list ap)
{
	fprintf(fd, "maus: ");
	vfprintf(fd, fmt, ap);
}

void maus_die(const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlog(stderr, fmt, ap);
	va_end(ap);

	exit(EXIT_FAILURE);
}

u64 maus_get_time_ns(void)
{
#if !defined(_WIN32)
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
#else
	static LARGE_INTEGER frequency = {0};
	LARGE_INTEGER counter;
	u64 sec;
	u64 rem;

	if (frequency.QuadPart == 0)
		QueryPerformanceFrequency(&frequency);

	QueryPerformanceCounter(&counter);

	sec = counter.QuadPart / frequency.QuadPart;
	rem = counter.QuadPart % frequency.QuadPart;

	return (sec * 1000000000) + ((rem * 1000000000) / frequency.QuadPart);
#endif
}

void maus_log(FILE* fd, const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlog(fd, fmt, ap);
	fputc('\n', fd);
	va_end(ap);
}

void maus_sleep(u32 ms)
{
#if !defined(_WIN32)
	struct timespec ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, NULL);
#else
	Sleep((DWORD)ms);
#endif
}

void maus_target_fps(Maus* mw, u32 fps)
{
	u64 ns_frame;
	u64 cur_time;
	u64 elapsed;
	u64 sleep;

	if (fps == 0)
		return;

	ns_frame = 1000000000 / fps; /* max ns a frame can take */
	cur_time = maus_get_time_ns();
	elapsed = cur_time - mw->frame_time_last;

	/* if early finish, sleep for the remaining time balance */
	if (elapsed < ns_frame) {
		sleep = ns_frame - elapsed;
		maus_sleep((u32)((sleep + 999999) / 1000000));
	}

	mw->frame_time_last = maus_get_time_ns();
}

