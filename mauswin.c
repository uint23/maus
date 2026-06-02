#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "mauswin.h"

static void vlog(FILE* fd, const char* fmt, va_list ap);

/* log message to output `fd` */
static void vlog(FILE* fd, const char* fmt, va_list ap)
{
	fprintf(fd, "maus: ");
	vfprintf(fd, fmt, ap);
	fflush(fd);
}

void maus_die(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vlog(stderr, fmt, ap);
	va_end(ap);

	exit(EXIT_FAILURE);
}

void maus_log(FILE* fd, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vlog(fd, fmt, ap);
	va_end(ap);
}

