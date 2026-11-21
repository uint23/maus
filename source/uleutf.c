#include <stddef.h>

#include "uletypes.h"
#include "uleutf.h"

int uleutf_casecmp(const char* src1, const char* src2)
{
	ule_i32 src1_lwr_cp = 0;
	ule_i32 src2_lwr_cp = 0;
	ule_i32 src1_upr_cp = 0;
	ule_i32 src2_upr_cp = 0;
	ule_i32 src1_orig_cp = 0;
	ule_i32 src2_orig_cp = 0;

	for (;;) {
		src1 = uleutf_codepoint(src1, &src1_orig_cp);
		src2 = uleutf_codepoint(src2, &src2_orig_cp);

		/* lower the srcs if required */
		src1_lwr_cp = uleutf_lwrcodepoint(src1_orig_cp);
		src2_lwr_cp = uleutf_lwrcodepoint(src2_orig_cp);

		/* lower the srcs if required */
		src1_upr_cp = uleutf_uprcodepoint(src1_orig_cp);
		src2_upr_cp = uleutf_uprcodepoint(src2_orig_cp);

		/* check if the lowered codepoints match */
		if ((0 == src1_orig_cp) && (0 == src2_orig_cp))
			return 0;
		else if ((src1_lwr_cp == src2_lwr_cp) || (src1_upr_cp == src2_upr_cp))
			continue;

		/* if they don't match, then we return the difference between the characters */
		return src1_lwr_cp - src2_lwr_cp;
	}
}

char* uleutf_cat(char* dst, const char* src)
{
	char* d = dst;

	/* find the null terminating byte in dst */
	while ('\0' != *d)
		d++;

	/* overwriting the null terminating byte in dst, append src byte-by-byte */
	while ('\0' != *src)
		*d++ = *src++;

	/* write out a new null terminating byte into dst */
	*d = '\0';

	return dst;
}

char* uleutf_chr(const char* src, ule_i32 chr)
{
	char c[5] = {'\0', '\0', '\0', '\0', '\0'};

	if (0 == chr) {
		/* being asked to return position of null terminating byte, so
		   just run s to the end, and return! */
		while ('\0' != *src)
			src++;

		return (char*)src;
	}
	else if (0 == ((ule_i32)0xffffff80 & chr)) {
		/* 1-byte/7-bit ascii
		   (0b0xxxxxxx) */
		c[0] = (ule_i8)chr;
	}
	else if (0 == ((ule_i32)0xfffff800 & chr)) {
		/* 2-byte/11-bit utf8 code point
		   (0b110xxxxx 0b10xxxxxx) */
		c[0] = (ule_i8)(0xc0 | (ule_i8)(chr >> 6));
		c[1] = (ule_i8)(0x80 | (ule_i8)(chr & 0x3f));
	}
	else if (0 == ((ule_i32)0xffff0000 & chr)) {
		/* 3-byte/16-bit utf8 code point
		   (0b1110xxxx 0b10xxxxxx 0b10xxxxxx) */
		c[0] = (ule_i8)(0xe0 | (ule_i8)(chr >> 12));
		c[1] = (ule_i8)(0x80 | (ule_i8)((chr >> 6) & 0x3f));
		c[2] = (ule_i8)(0x80 | (ule_i8)(chr & 0x3f));
	}
	else { /* if (0 == ((int)0xffe00000 & chr)) { */
		/* 4-byte/21-bit utf8 code point
		   (0b11110xxx 0b10xxxxxx 0b10xxxxxx 0b10xxxxxx) */
		c[0] = (ule_i8)(0xf0 | (ule_i8)(chr >> 18));
		c[1] = (ule_i8)(0x80 | (ule_i8)((chr >> 12) & 0x3f));
		c[2] = (ule_i8)(0x80 | (ule_i8)((chr >> 6) & 0x3f));
		c[3] = (ule_i8)(0x80 | (ule_i8)(chr & 0x3f));
	}

	/* we've made c into a 2 utf8 codepoint string, one for the chr we are
	   seeking, another for the null terminating byte. now use utf8str to search */
	return uleutf_str(src, c);
}

int uleutf_cmp(const char* src1, const char* src2)
{
	while (('\0' != *src1) || ('\0' != *src2)) {
		if ((unsigned char)*src1 < (unsigned char)*src2)
			return -1;
		else if ((unsigned char)*src1 > (unsigned char)*src2)
			return 1;

		src1++;
		src2++;
	}

	/* both utf8 strings matched */
	return 0;
}

int uleutf_ncmp(const char* src1, const char* src2, size_t n)
{
	while ((0 != n--) && (('\0' != *src1) || ('\0' != *src2))) {
		if ((unsigned char)*src1 < (unsigned char)*src2)
			return -1;
		else if ((unsigned char)*src1 > (unsigned char)*src2)
			return 1;

		src1++;
		src2++;
	}

	/* both utf8 strings matched */
	return 0;
}

size_t uleutf_len(const char* str)
{
	return uleutf_nlen(str, ULE_USIZE_MAX);
}

size_t uleutf_nlen(const char* str, size_t n)
{
	const ule_i8* t = (ule_i8*)str;
	size_t length = 0;

	while ((size_t)(str - ((const char*)t)) < n && '\0' != *str) {
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
		while (--cp_len && (size_t)(str - ((const char*)t)) < n && '\0' != *str);

		if (cp_len && (size_t)(str - ((const char*)t)) == n)
			break;

		/* no matter the bytes we marched s forward by, it was
		   only 1 utf8 codepoint */
		length++;
	}

	return length;
}

char* uleutf_str(const char* haystack, const char* needle)
{
	ule_i32 throwaway_codepoint = 0;

	/* if needle has no utf8 codepoints before the null terminating
	   byte then return haystack */
	if ('\0' == *needle)
		return (char*)haystack;

	while ('\0' != *haystack) {
		const char* maybe_match = haystack;
		const char* n = needle;

		while (*haystack == *n && (*haystack != '\0' && *n != '\0')) {
			n++;
			haystack++;
		}

		if ('\0' == *n)
			/* we found the whole utf8 string for needle in haystack at
			   maybeMatch, so return it */
			return (char*)maybe_match;
		else
			/* h could be in the middle of an unmatching utf8 codepoint,
			   so we need to march it on to the next character beginning
			   starting from the current character */
			haystack = uleutf_codepoint(maybe_match, &throwaway_codepoint);
	}

	/* no match */
	return NULL;
}

char* uleutf_valid(const char* str)
{
	return uleutf_nvalid(str, ULE_USIZE_MAX);
}

char* uleutf_nvalid(const char* str, size_t n)
{
	const ule_i8* t = (ule_i8*)str;
	size_t consumed = 0;
	size_t remaining = 0;

	while ((void)(consumed = (size_t)(str - (const char*)t)), consumed < n && '\0' != *str) {
		remaining = n - consumed;

		if (0x00 == (0x80 & *str))
			str += 1; /* 1-byte ascii (began with 0b0xxxxxxx) */

		else if (0xf0 == (0xf8 & *str)) {
			/* ensure that there's 4 bytes or more remaining */
			if (remaining < 4)
				return (char*)str;

			/* ensure each of the 3 following bytes in this 4-byte
			   utf8 codepoint began with 0b10xxxxxx */
			if ((0x80 != (0xc0 & str[1])) || (0x80 != (0xc0 & str[2])) || (0x80 != (0xc0 & str[3])))
				return (char*)str;

			/* ensure that our utf8 codepoint ended after 4 bytes */
			if ((remaining != 4) && (0x80 == (0xc0 & str[4])))
				return (char*)str;

			/* ensure that the top 5 bits of this 4-byte utf8
			   codepoint were not 0, as then we could have used
			   one of the smaller encodings */
			if ((0 == (0x07 & str[0])) && (0 == (0x30 & str[1])))
				return (char*)str;

			/* ensure that the codepoint is not above U+10FFFF */
			if (((0x07 & str[0]) > 4) || ((4 == (0x07 & str[0])) && (0 != (0x30 & str[1]))))
				return (char*)str;

			/* 4-byte utf8 code point (began with 0b11110xxx) */
			str += 4;
		}

		else if (0xe0 == (0xf0 & *str)) {
			/* ensure that there's 3 bytes or more remaining */
			if (remaining < 3)
				return (char*)str;

			/* ensure each of the 2 following bytes in this 3-byte
			   utf8 codepoint began with 0b10xxxxxx */
			if ((0x80 != (0xc0 & str[1])) || (0x80 != (0xc0 & str[2])))
				return (char*)str;

			/* ensure that our utf8 codepoint ended after 3 bytes */
			if ((remaining != 3) && (0x80 == (0xc0 & str[3])))
				return (char*)str;

			/* ensure that the top 5 bits of this 3-byte utf8
			   codepoint were not 0, as then we could have used
			   one of the smaller encodings */
			if ((0 == (0x0f & str[0])) && (0 == (0x20 & str[1])))
				return (char*)str;

			/* ensure that the codepoint is not a surrogate (U+D800..U+DFFF) */
			if ((0x0d == (0x0f & str[0])) && (0 != (0x20 & str[1])))
				return (char*)str;

			/* 3-byte utf8 code point (began with 0b1110xxxx) */
			str += 3;
		}

		else if (0xc0 == (0xe0 & *str)) {
			/* ensure that there's 2 bytes or more remaining */
			if (remaining < 2)
				return (char*)str;

			/* ensure the 1 following byte in this 2-byte
			   utf8 codepoint began with 0b10xxxxxx */
			if (0x80 != (0xc0 & str[1]))
				return (char*)str;

			/* ensure that our utf8 codepoint ended after 2 bytes */
			if ((remaining != 2) && (0x80 == (0xc0 & str[2])))
				return (char*)str;

			/* ensure that the top 4 bits of this 2-byte utf8
			   codepoint were not 0, as then we could have used
			   one of the smaller encodings */
			if (0 == (0x1e & str[0]))
				return (char*)str;

			/* 2-byte utf8 code point (began with 0b110xxxxx) */
			str += 2;
		}
		else
			return (char*)str; /* we have an invalid 0b1xxxxxxx utf8 code point entry */
	}

	return NULL;
}

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

ule_i32 uleutf_lwrcodepoint(ule_i32 cp)
{
	if (((0x0041 <= cp) && (0x005a >= cp)) ||
	    ((0x00c0 <= cp) && (0x00d6 >= cp)) ||
	    ((0x00d8 <= cp) && (0x00de >= cp)) ||
	    ((0x0391 <= cp) && (0x03a1 >= cp)) ||
	    ((0x03a3 <= cp) && (0x03ab >= cp)) ||
	    ((0x0410 <= cp) && (0x042f >= cp)))
		cp += 32;
	else if ((0x0400 <= cp) && (0x040f >= cp))
		cp += 80;
	else if ((0x0531 <= cp) && (0x0556 >= cp))
	      cp += 48; /* Armenian */
	else if ((0x13a0 <= cp) && (0x13ef >= cp))
	      cp += 0x97d0; /* Cherokee to Cherokee Supplement */
	else if ((0x13f0 <= cp) && (0x13f5 >= cp))
	      cp += 8;
	else if (((0x0100 <= cp) && (0x012f >= cp)) ||
	         ((0x0132 <= cp) && (0x0137 >= cp)) ||
	         ((0x014a <= cp) && (0x0177 >= cp)) ||
	         ((0x0182 <= cp) && (0x0185 >= cp)) ||
	         ((0x01a0 <= cp) && (0x01a5 >= cp)) ||
	         ((0x01de <= cp) && (0x01ef >= cp)) ||
	         ((0x01f8 <= cp) && (0x021f >= cp)) ||
	         ((0x0222 <= cp) && (0x0233 >= cp)) ||
	         ((0x0246 <= cp) && (0x024f >= cp)) ||
	         ((0x03d8 <= cp) && (0x03ef >= cp)) ||
	         ((0x0460 <= cp) && (0x0481 >= cp)) ||
	         ((0x048a <= cp) && (0x04bf >= cp)) ||
	         ((0x04d0 <= cp) && (0x04ff >= cp)) ||
	         /* Cyrillic Supplement: even codepoints are uppercase */
	         ((0x0500 <= cp) && (0x052e >= cp)) ||
	         /* Latin Extended-C: even=upper ranges */
	         ((0x2c60 <= cp) && (0x2c61 >= cp)) ||
	         ((0x2c72 <= cp) && (0x2c73 >= cp)) ||
	         /* Latin Extended-D: even=upper ranges */
	         ((0xa722 <= cp) && (0xa72f >= cp)) ||
	         ((0xa732 <= cp) && (0xa76f >= cp)) ||
	         ((0xa77e <= cp) && (0xa787 >= cp)) ||
	         ((0xa790 <= cp) && (0xa793 >= cp)) ||
	         ((0xa796 <= cp) && (0xa7a9 >= cp)) ||
	         ((0xa7b4 <= cp) && (0xa7c3 >= cp)))
		cp |= 0x1;
	else if (((0x0139 <= cp) && (0x0148 >= cp)) ||
	         ((0x0179 <= cp) && (0x017e >= cp)) ||
	         ((0x01af <= cp) && (0x01b0 >= cp)) ||
	         ((0x01b3 <= cp) && (0x01b6 >= cp)) ||
	         ((0x01cd <= cp) && (0x01dc >= cp)) ||
	         ((0x04c1 <= cp) && (0x04ce >= cp))) {
		cp += 1;
		cp &= ~0x1;
	}
	else {
		switch (cp) {
		default: break;
		case 0x0178: cp = 0x00ff; break;
		case 0x04c0: cp = 0x04cf; break;
		case 0x0243: cp = 0x0180; break;
		case 0x018e: cp = 0x01dd; break;
		case 0x023d: cp = 0x019a; break;
		case 0x0220: cp = 0x019e; break;
		case 0x01b7: cp = 0x0292; break;
		case 0x01c4: cp = 0x01c6; break;
		case 0x01c7: cp = 0x01c9; break;
		case 0x01ca: cp = 0x01cc; break;
		case 0x01f1: cp = 0x01f3; break;
		case 0x01f7: cp = 0x01bf; break;
		case 0x0187: cp = 0x0188; break;
		case 0x018b: cp = 0x018c; break;
		case 0x0191: cp = 0x0192; break;
		case 0x0198: cp = 0x0199; break;
		case 0x01a7: cp = 0x01a8; break;
		case 0x01ac: cp = 0x01ad; break;
		case 0x01b8: cp = 0x01b9; break;
		case 0x01bc: cp = 0x01bd; break;
		case 0x01f4: cp = 0x01f5; break;
		case 0x023b: cp = 0x023c; break;
		case 0x0241: cp = 0x0242; break;
		case 0x03fd: cp = 0x037b; break;
		case 0x03fe: cp = 0x037c; break;
		case 0x03ff: cp = 0x037d; break;
		case 0x037f: cp = 0x03f3; break;
		case 0x0386: cp = 0x03ac; break;
		case 0x0388: cp = 0x03ad; break;
		case 0x0389: cp = 0x03ae; break;
		case 0x038a: cp = 0x03af; break;
		case 0x038c: cp = 0x03cc; break;
		case 0x038e: cp = 0x03cd; break;
		case 0x038f: cp = 0x03ce; break;
		case 0x0370: cp = 0x0371; break;
		case 0x0372: cp = 0x0373; break;
		case 0x0376: cp = 0x0377; break;
		case 0x03f4: cp = 0x03b8; break;
		case 0x03cf: cp = 0x03d7; break;
		case 0x03f9: cp = 0x03f2; break;
		case 0x03f7: cp = 0x03f8; break;
		case 0x03fa: cp = 0x03fb; break;
		/* Latin Extended-C individual mappings */
		case 0x2c60: cp = 0x2c61; break;
		case 0x2c62: cp = 0x026b; break;
		case 0x2c63: cp = 0x1d7d; break;
		case 0x2c64: cp = 0x027d; break;
		/* Latin Extended-C odd=upper pairs (upper odd -> lower even = upper+1) */
		case 0x2c67: cp = 0x2c68; break;
		case 0x2c69: cp = 0x2c6a; break;
		case 0x2c6b: cp = 0x2c6c; break;
		case 0x2c75: cp = 0x2c76; break;
		case 0x2c6d: cp = 0x0251; break;
		case 0x2c6e: cp = 0x0271; break;
		case 0x2c6f: cp = 0x0250; break;
		case 0x2c70: cp = 0x0252; break;
		/* Latin Extended-D individual mappings */
		/* Odd=upper pairs: A779->A77A, A77B->A77C, A78B->A78C */
		case 0xa779: cp = 0xa77a; break;
		case 0xa77b: cp = 0xa77c; break;
		case 0xa78b: cp = 0xa78c; break;
		case 0xa77d: cp = 0x1d79; break;
		case 0xa78d: cp = 0x0265; break;
		case 0xa7aa: cp = 0x0266; break;
		case 0xa7ab: cp = 0x025c; break;
		case 0xa7ac: cp = 0x0261; break;
		case 0xa7ad: cp = 0x026c; break;
		case 0xa7ae: cp = 0x026a; break;
		case 0xa7b0: cp = 0x029e; break;
		case 0xa7b1: cp = 0x0287; break;
		case 0xa7b2: cp = 0x029d; break;
		case 0xa7b3: cp = 0xab53; break;
		/* U+1C80..U+1C88 are historic lowercase Cyrillic forms; already lowercase */
		}
	}

	return cp;
}

ule_i32 uleutf_uprcodepoint(ule_i32 cp)
{
	if (((0x0061 <= cp) && (0x007a >= cp)) ||
	    ((0x00e0 <= cp) && (0x00f6 >= cp)) ||
	    ((0x00f8 <= cp) && (0x00fe >= cp)) ||
	    ((0x03b1 <= cp) && (0x03c1 >= cp)) ||
	    ((0x03c3 <= cp) && (0x03cb >= cp)) ||
	    ((0x0430 <= cp) && (0x044f >= cp)))
		cp -= 32;
	else if ((0x0450 <= cp) && (0x045f >= cp))
		cp -= 80;
	else if ((0x0561 <= cp) && (0x0586 >= cp))
		cp -= 48; /* Armenian */
	else if ((0xab70 <= cp) && (0xabbf >= cp))
		cp -= 0x97d0; /* Cherokee Supplement to Cherokee */
	else if ((0x13f8 <= cp) && (0x13fd >= cp))
		cp -= 8;
	else if (((0x0100 <= cp) && (0x012f >= cp)) ||
	         ((0x0132 <= cp) && (0x0137 >= cp)) ||
	         ((0x014a <= cp) && (0x0177 >= cp)) ||
	         ((0x0182 <= cp) && (0x0185 >= cp)) ||
	         ((0x01a0 <= cp) && (0x01a5 >= cp)) ||
	         ((0x01de <= cp) && (0x01ef >= cp)) ||
	         ((0x01f8 <= cp) && (0x021f >= cp)) ||
	         ((0x0222 <= cp) && (0x0233 >= cp)) ||
	         ((0x0246 <= cp) && (0x024f >= cp)) ||
	         ((0x03d8 <= cp) && (0x03ef >= cp)) ||
	         ((0x0460 <= cp) && (0x0481 >= cp)) ||
	         ((0x048a <= cp) && (0x04bf >= cp)) ||
	         ((0x04d0 <= cp) && (0x04ff >= cp)) ||
	         /* Cyrillic Supplement: odd codepoints are lowercase (even=upper) */
	         ((0x0501 <= cp) && (0x052f >= cp)) ||
	         /* Latin Extended-C: odd=lower ranges (even=upper) */
	         ((0x2c73 <= cp) && (0x2c73 >= cp)) ||
	         /* Latin Extended-D: odd=lower ranges (even=upper) */
	         ((0xa722 <= cp) && (0xa72f >= cp)) ||
	         ((0xa732 <= cp) && (0xa76f >= cp)) ||
	         ((0xa77e <= cp) && (0xa787 >= cp)) ||
	         ((0xa790 <= cp) && (0xa793 >= cp)) ||
	         ((0xa796 <= cp) && (0xa7a9 >= cp)) ||
	         ((0xa7b4 <= cp) && (0xa7c3 >= cp))) {
				cp &= ~0x1;
			}
	else if (((0x0139 <= cp) && (0x0148 >= cp)) ||
	         ((0x0179 <= cp) && (0x017e >= cp)) ||
	         ((0x01af <= cp) && (0x01b0 >= cp)) ||
	         ((0x01b3 <= cp) && (0x01b6 >= cp)) ||
	         ((0x01cd <= cp) && (0x01dc >= cp)) ||
	         ((0x04c1 <= cp) && (0x04ce >= cp))) {
		cp -= 1;
		cp |= 0x1;
	}
	else {
		switch (cp) {
		default: break;
		case 0x00ff: cp = 0x0178; break;
		case 0x04cf: cp = 0x04c0; break;
		case 0x03c2:
			/* final sigma has no uppercase form of its own */
			cp = 0x03a3;
			break;
		case 0x0180: cp = 0x0243; break;
		case 0x01dd: cp = 0x018e; break;
		case 0x019a: cp = 0x023d; break;
		case 0x019e: cp = 0x0220; break;
		case 0x0292: cp = 0x01b7; break;
		case 0x01c6: cp = 0x01c4; break;
		case 0x01c9: cp = 0x01c7; break;
		case 0x01cc: cp = 0x01ca; break;
		case 0x01f3: cp = 0x01f1; break;
		case 0x01bf: cp = 0x01f7; break;
		case 0x0188: cp = 0x0187; break;
		case 0x018c: cp = 0x018b; break;
		case 0x0192: cp = 0x0191; break;
		case 0x0199: cp = 0x0198; break;
		case 0x01a8: cp = 0x01a7; break;
		case 0x01ad: cp = 0x01ac; break;
		case 0x01b9: cp = 0x01b8; break;
		case 0x01bd: cp = 0x01bc; break;
		case 0x01f5: cp = 0x01f4; break;
		case 0x023c: cp = 0x023b; break;
		case 0x0242: cp = 0x0241; break;
		case 0x037b: cp = 0x03fd; break;
		case 0x037c: cp = 0x03fe; break;
		case 0x037d: cp = 0x03ff; break;
		case 0x03f3: cp = 0x037f; break;
		case 0x03ac: cp = 0x0386; break;
		case 0x03ad: cp = 0x0388; break;
		case 0x03ae: cp = 0x0389; break;
		case 0x03af: cp = 0x038a; break;
		case 0x03cc: cp = 0x038c; break;
		case 0x03cd: cp = 0x038e; break;
		case 0x03ce: cp = 0x038f; break;
		case 0x0371: cp = 0x0370; break;
		case 0x0373: cp = 0x0372; break;
		case 0x0377: cp = 0x0376; break;
		case 0x03d1: cp = 0x0398; break;
		case 0x03d7: cp = 0x03cf; break;
		case 0x03f2: cp = 0x03f9; break;
		case 0x03f8: cp = 0x03f7; break;
		case 0x03fb: cp = 0x03fa; break;
		/* Latin Extended-C individual reverse mappings (lowercase -> uppercase) */
		case 0x2c61: cp = 0x2c60; break;
		/* Latin Extended-C even=lower (odd=upper) pairs: upr(even) = even-1 */
		case 0x2c68: cp = 0x2c67; break;
		case 0x2c6a: cp = 0x2c69; break;
		case 0x2c6c: cp = 0x2c6b; break;
		case 0x2c73: cp = 0x2c72; break;
		case 0x2c76: cp = 0x2c75; break;
		case 0x026b: cp = 0x2c62; break;
		case 0x1d7d: cp = 0x2c63; break;
		case 0x027d: cp = 0x2c64; break;
		case 0x0251: cp = 0x2c6d; break;
		case 0x0271: cp = 0x2c6e; break;
		case 0x0250: cp = 0x2c6f; break;
		case 0x0252: cp = 0x2c70; break;
		/* Latin Extended-D individual reverse mappings */
		/* Even=lower (odd=upper) pairs: upr(even) = even-1 */
		case 0xa77a: cp = 0xa779; break;
		case 0xa77c: cp = 0xa77b; break;
		case 0xa78c: cp = 0xa78b; break;
		case 0x1d79: cp = 0xa77d; break;
		case 0x0265: cp = 0xa78d; break;
		case 0x0266: cp = 0xa7aa; break;
		case 0x025c: cp = 0xa7ab; break;
		case 0x0261: cp = 0xa7ac; break;
		case 0x026c: cp = 0xa7ad; break;
		case 0x026a: cp = 0xa7ae; break;
		case 0x029e: cp = 0xa7b0; break;
		case 0x0287: cp = 0xa7b1; break;
		case 0x029d: cp = 0xa7b2; break;
		case 0xab53: cp = 0xa7b3; break;
		/* Cyrillic Extended-C: historic lowercase -> Cyrillic uppercase */
		case 0x1c80: cp = 0x0412; break;
		case 0x1c81: cp = 0x0414; break;
		case 0x1c82: cp = 0x041e; break;
		case 0x1c83: cp = 0x0421; break;
		case 0x1c84:
		case 0x1c85: cp = 0x0422; break;
		case 0x1c86: cp = 0x042a; break;
		case 0x1c87: cp = 0x0462; break;
		case 0x1c88: cp = 0xa64a; break;
		}
	}

	return cp;
}

