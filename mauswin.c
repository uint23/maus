#include <stdarg.h>
#include <stdio.h>

#include "mauswin.h"

/* log message to output `fd` */
void maus_log(FILE* fd, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(fd, fmt, ap);
	fflush(stderr);
	va_end(ap);
}

