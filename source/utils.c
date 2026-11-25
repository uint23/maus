#include <stdlib.h>
#include <string.h>

#include "uletypes.h"
#include "utils.h"

ule_u32 maus_blend(ule_u32 src, ule_u32 dst)
{
	ule_u32 a = src >> 24;
	ule_u32 inv = 255 - a;
	ule_u32 r;
	ule_u32 g;
	ule_u32 b;

	r = (((src >> 16) & 255) * a + ((dst >> 16) & 255) * inv + 127) / 255;
	g = (((src >> 8)  & 255) * a + ((dst >> 8)  & 255) * inv + 127) / 255;
	b = ((src & 255)         * a + (dst & 255)         * inv + 127) / 255;

	return 0xff000000u | (r << 16) | (g << 8) | b;
}

char* maus_strdup(const char* src)
{
	size_t len;
	char* dst;
	if (src == NULL)
		return NULL;

	len = strlen(src);
	dst = malloc(len + 1);
	if (dst == NULL)
		return NULL;

	memcpy(dst, src, len + 1);
	return dst;
}

