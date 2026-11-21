/* Copies of libule's codepoint and length functions (ANSI C port of utf8.h). */

#include <stddef.h>

#include "uleutf.h"

char* uleutf_codepoint(const char* str, ule_i32* out_codepoint)
{
	size_t n = 1;
	size_t i = 1;
	ule_i32 cp = (ule_i32)str[0];

	if (0xf0 == (0xf8 & str[0])) {
		/* 4 byte utf8 codepoint */
		n = 4;
		cp = 0x07 & str[0];
	}
	else if (0xe0 == (0xf0 & str[0])) {
		/* 3 byte utf8 codepoint */
		n = 3;
		cp = 0x0f & str[0];
	}
	else if (0xc0 == (0xe0 & str[0])) {
		/* 2 byte utf8 codepoint */
		n = 2;
		cp = 0x1f & str[0];
	}

	/* stop at the NUL if the sequence is truncated */
	for (i = 1; i < n && '\0' != str[i]; i++)
		cp = (cp << 6) | (0x3f & str[i]);

	*out_codepoint = cp;
	return (char*)(str + i);
}

size_t uleutf_codepointcalcsize(const char* str)
{
	if (0xf0 == (0xf8 & str[0]))
		return 4; /* 4 byte utf8 codepoint */
	else if (0xe0 == (0xf0 & str[0]))
		return 3; /* 3 byte utf8 codepoint */
	else if (0xc0 == (0xe0 & str[0]))
		return 2; /* 2 byte utf8 codepoint */

	return 1; /* 1 byte utf8 codepoint otherwise */
}

size_t uleutf_codepointsize(ule_i32 chr)
{
	if (0 == ((ule_i32)0xffffff80 & chr))
		return 1;
	else if (0 == ((ule_i32)0xfffff800 & chr))
		return 2;
	else if (0 == ((ule_i32)0xffff0000 & chr))
		return 3;
	else /* if (0 == ((int)0xffe00000 & chr)) */
		return 4;
}

int uleutf_codepointvisualwidth(ule_i32 chr)
{
	/* the null codepoint takes up no visual space */
	if (0 == chr)
		return 0;

	/* C0 and C1 control codepoints have no printable representation. */
	if ((chr < 0x20) || ((0x7f <= chr) && (chr < 0xa0)))
		return -1;

	/* zero-width combining marks and format codepoints. the ranges follow the
	   widely used wcwidth() tables and cover Unicode general categories Mn, Me
	   and Cf that do not advance the cursor */
	if (((0x0300 <= chr) && (0x036f >= chr)) ||
	    ((0x0483 <= chr) && (0x0486 >= chr)) ||
	    ((0x0488 <= chr) && (0x0489 >= chr)) ||
	    ((0x0591 <= chr) && (0x05bd >= chr)) || (0x05bf == chr) ||
	    ((0x05c1 <= chr) && (0x05c2 >= chr)) ||
	    ((0x05c4 <= chr) && (0x05c5 >= chr)) || (0x05c7 == chr) ||
	    ((0x0600 <= chr) && (0x0603 >= chr)) ||
	    ((0x0610 <= chr) && (0x0615 >= chr)) ||
	    ((0x064b <= chr) && (0x065e >= chr)) || (0x0670 == chr) ||
	    ((0x06d6 <= chr) && (0x06e4 >= chr)) ||
	    ((0x06e7 <= chr) && (0x06e8 >= chr)) ||
	    ((0x06ea <= chr) && (0x06ed >= chr)) || (0x070f == chr) ||
	    (0x0711 == chr) || ((0x0730 <= chr) && (0x074a >= chr)) ||
	    ((0x07a6 <= chr) && (0x07b0 >= chr)) ||
	    ((0x07eb <= chr) && (0x07f3 >= chr)) ||
	    ((0x0901 <= chr) && (0x0902 >= chr)) || (0x093c == chr) ||
	    ((0x0941 <= chr) && (0x0948 >= chr)) || (0x094d == chr) ||
	    ((0x0951 <= chr) && (0x0954 >= chr)) ||
	    ((0x0962 <= chr) && (0x0963 >= chr)) || (0x0981 == chr) ||
	    (0x09bc == chr) || ((0x09c1 <= chr) && (0x09c4 >= chr)) ||
	    (0x09cd == chr) || ((0x09e2 <= chr) && (0x09e3 >= chr)) ||
	    ((0x0a01 <= chr) && (0x0a02 >= chr)) || (0x0a3c == chr) ||
	    ((0x0a41 <= chr) && (0x0a42 >= chr)) ||
	    ((0x0a47 <= chr) && (0x0a48 >= chr)) ||
	    ((0x0a4b <= chr) && (0x0a4d >= chr)) ||
	    ((0x0a70 <= chr) && (0x0a71 >= chr)) ||
	    ((0x0a81 <= chr) && (0x0a82 >= chr)) || (0x0abc == chr) ||
	    ((0x0ac1 <= chr) && (0x0ac5 >= chr)) ||
	    ((0x0ac7 <= chr) && (0x0ac8 >= chr)) || (0x0acd == chr) ||
	    ((0x0ae2 <= chr) && (0x0ae3 >= chr)) || (0x0b01 == chr) ||
	    (0x0b3c == chr) || (0x0b3f == chr) ||
	    ((0x0b41 <= chr) && (0x0b43 >= chr)) || (0x0b4d == chr) ||
	    (0x0b56 == chr) || (0x0b82 == chr) || (0x0bc0 == chr) ||
	    (0x0bcd == chr) || ((0x0c3e <= chr) && (0x0c40 >= chr)) ||
	    ((0x0c46 <= chr) && (0x0c48 >= chr)) ||
	    ((0x0c4a <= chr) && (0x0c4d >= chr)) ||
	    ((0x0c55 <= chr) && (0x0c56 >= chr)) || (0x0cbc == chr) ||
	    (0x0cbf == chr) || (0x0cc6 == chr) ||
	    ((0x0ccc <= chr) && (0x0ccd >= chr)) ||
	    ((0x0ce2 <= chr) && (0x0ce3 >= chr)) ||
	    ((0x0d41 <= chr) && (0x0d43 >= chr)) || (0x0d4d == chr) ||
	    (0x0dca == chr) || ((0x0dd2 <= chr) && (0x0dd4 >= chr)) ||
	    (0x0dd6 == chr) || (0x0e31 == chr) ||
	    ((0x0e34 <= chr) && (0x0e3a >= chr)) ||
	    ((0x0e47 <= chr) && (0x0e4e >= chr)) || (0x0eb1 == chr) ||
	    ((0x0eb4 <= chr) && (0x0eb9 >= chr)) ||
	    ((0x0ebb <= chr) && (0x0ebc >= chr)) ||
	    ((0x0ec8 <= chr) && (0x0ecd >= chr)) ||
	    ((0x0f18 <= chr) && (0x0f19 >= chr)) || (0x0f35 == chr) ||
	    (0x0f37 == chr) || (0x0f39 == chr) ||
	    ((0x0f71 <= chr) && (0x0f7e >= chr)) ||
	    ((0x0f80 <= chr) && (0x0f84 >= chr)) ||
	    ((0x0f86 <= chr) && (0x0f87 >= chr)) ||
	    ((0x0f90 <= chr) && (0x0f97 >= chr)) ||
	    ((0x0f99 <= chr) && (0x0fbc >= chr)) || (0x0fc6 == chr) ||
	    ((0x102d <= chr) && (0x1030 >= chr)) || (0x1032 == chr) ||
	    ((0x1036 <= chr) && (0x1037 >= chr)) || (0x1039 == chr) ||
	    ((0x1058 <= chr) && (0x1059 >= chr)) ||
	    ((0x1160 <= chr) && (0x11ff >= chr)) || (0x135f == chr) ||
	    ((0x1712 <= chr) && (0x1714 >= chr)) ||
	    ((0x1732 <= chr) && (0x1734 >= chr)) ||
	    ((0x1752 <= chr) && (0x1753 >= chr)) ||
	    ((0x1772 <= chr) && (0x1773 >= chr)) ||
	    ((0x17b4 <= chr) && (0x17b5 >= chr)) ||
	    ((0x17b7 <= chr) && (0x17bd >= chr)) || (0x17c6 == chr) ||
	    ((0x17c9 <= chr) && (0x17d3 >= chr)) || (0x17dd == chr) ||
	    ((0x180b <= chr) && (0x180d >= chr)) || (0x18a9 == chr) ||
	    ((0x1920 <= chr) && (0x1922 >= chr)) ||
	    ((0x1927 <= chr) && (0x1928 >= chr)) || (0x1932 == chr) ||
	    ((0x1939 <= chr) && (0x193b >= chr)) ||
	    ((0x1a17 <= chr) && (0x1a18 >= chr)) ||
	    ((0x1b00 <= chr) && (0x1b03 >= chr)) || (0x1b34 == chr) ||
	    ((0x1b36 <= chr) && (0x1b3a >= chr)) || (0x1b3c == chr) ||
	    (0x1b42 == chr) || ((0x1b6b <= chr) && (0x1b73 >= chr)) ||
	    ((0x1dc0 <= chr) && (0x1dca >= chr)) ||
	    ((0x1dfe <= chr) && (0x1dff >= chr)) ||
	    ((0x200b <= chr) && (0x200f >= chr)) ||
	    ((0x202a <= chr) && (0x202e >= chr)) ||
	    ((0x2060 <= chr) && (0x2063 >= chr)) ||
	    ((0x206a <= chr) && (0x206f >= chr)) ||
	    ((0x20d0 <= chr) && (0x20ef >= chr)) ||
	    ((0x302a <= chr) && (0x302f >= chr)) ||
	    ((0x3099 <= chr) && (0x309a >= chr)) || (0xa806 == chr) ||
	    (0xa80b == chr) || ((0xa825 <= chr) && (0xa826 >= chr)) ||
	    (0xfb1e == chr) || ((0xfe00 <= chr) && (0xfe0f >= chr)) ||
	    ((0xfe20 <= chr) && (0xfe23 >= chr)) || (0xfeff == chr) ||
	    ((0xfff9 <= chr) && (0xfffb >= chr)) ||
	    ((0x10a01 <= chr) && (0x10a03 >= chr)) ||
	    ((0x10a05 <= chr) && (0x10a06 >= chr)) ||
	    ((0x10a0c <= chr) && (0x10a0f >= chr)) ||
	    ((0x10a38 <= chr) && (0x10a3a >= chr)) || (0x10a3f == chr) ||
	    ((0x1d167 <= chr) && (0x1d169 >= chr)) ||
	    ((0x1d173 <= chr) && (0x1d182 >= chr)) ||
	    ((0x1d185 <= chr) && (0x1d18b >= chr)) ||
	    ((0x1d1aa <= chr) && (0x1d1ad >= chr)) ||
	    ((0x1d242 <= chr) && (0x1d244 >= chr)) || (0xe0001 == chr) ||
	    ((0xe0020 <= chr) && (0xe007f >= chr)) ||
	    ((0xe0100 <= chr) && (0xe01ef >= chr)))
		return 0;

	/* wide (double-width) codepoints, mostly East Asian scripts */
	if ((0x1100 <= chr) &&
	    (((0x1100 <= chr) && (0x115f >= chr)) || (0x2329 == chr) ||
	     (0x232a == chr) ||
	     (((0x2e80 <= chr) && (0xa4cf >= chr)) && (0x303f != chr)) ||
	     ((0xac00 <= chr) && (0xd7a3 >= chr)) ||
	     ((0xf900 <= chr) && (0xfaff >= chr)) ||
	     ((0xfe10 <= chr) && (0xfe19 >= chr)) ||
	     ((0xfe30 <= chr) && (0xfe6f >= chr)) ||
	     ((0xff00 <= chr) && (0xff60 >= chr)) ||
	     ((0xffe0 <= chr) && (0xffe6 >= chr)) ||
	     ((0x20000 <= chr) && (0x2fffd >= chr)) ||
	     ((0x30000 <= chr) && (0x3fffd >= chr))))
		return 2;

	return 1;
}

size_t uleutf_len(const char* str)
{
	return uleutf_nlen(str, ULE_USIZE_MAX);
}

size_t uleutf_nlen(const char* str, size_t n)
{
	const char* t = str;
	size_t length = 0;

	while ((size_t)(str - t) < n && '\0' != *str) {
		size_t cp_len = 1;

		/* fast path: an ASCII byte is a whole codepoint */
		if (0 == (0x80 & *str)) {
			str++;
			length++;
			continue;
		}

		if (0xf0 == (0xf8 & *str))
			/* 4-byte utf8 code point (began with 0b11110xxx) */
			cp_len = 4;
		else if (0xe0 == (0xf0 & *str))
		      /* 3-byte utf8 code point (began with 0b1110xxxx) */
		      cp_len = 3;
		else if (0xc0 == (0xe0 & *str))
		      /* 2-byte utf8 code point (began with 0b110xxxxx) */
		      cp_len = 2;
		else /* if (0x00 == (0x80 & *s)) { */
		      /* 1-byte ascii (began with 0b0xxxxxxx) */
		      cp_len = 1;

		/* do not advance past the NUL or the n-byte limit */
		do
			str += 1;
		while (--cp_len && (size_t)(str - t) < n && '\0' != *str);

		if (cp_len && (size_t)(str - t) == n)
			break;

		/* no matter the bytes we marched s forward by, it was
		   only 1 utf8 codepoint */
		length++;
	}

	return length;
}
