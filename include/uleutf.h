/* Copyright 2026 uint (and contributors)

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

/* uleutf is based on utf8.h on https://github.com/sheredom/utf8.h
   Implementations are their work, not mine. This is just an ANSI C rewrite */

#ifndef ULEUTF_H
#define ULEUTF_H

#include "uletypes.h"

/* Return less than 0, 0, greater than 0 if src1 < src2, src1 == src2, src1 >
   src2 respectively, case insensitive */
int uleutf_casecmp(const char* src1, const char* src2);

/* Append the utf8 string src onto the utf8 string dst */
char* uleutf_cat(char* dst, const char* src);

/* Find the first match of the utf8 codepoint chr in the utf8 string src */
char* uleutf_chr(const char* src, ule_i32 chr);

/* Return less than 0, 0, greater than 0 if src1 < src2, src1 == src2,
   src1 > src2 respectively */
int uleutf_cmp(const char* src1, const char* src2);

/* Number of utf8 codepoints in the utf8 string str,
   excluding the null terminating byte */
size_t uleutf_len(const char* str);

/* Similar to utf8len, except that only at most n bytes of src are looked */
size_t uleutf_nlen(const char* str, size_t n);

/* Return less than 0, 0, greater than 0 if src1 < src2,
   src1 == src2, src1 > src2 respectively. Checking at most n
   bytes of each utf8 string */
int uleutf_ncmp(const char* src1, const char* src2, size_t n);

/* The position of the utf8 string needle in the utf8 string haystack */
char* uleutf_str(const char* haystack, const char* needle);

/* Return 0 on success, or the position of the invalid
   utf8 codepoint on failure */
char* uleutf_valid(const char* str);

/* Similar to utf8valid, except that only at most n bytes of src are looked */
char* uleutf_nvalid(const char* str, size_t n);

/* Sets out_codepoint to the current utf8 codepoint in str, and returns the
   address of the next utf8 codepoint after the current one in str */
char* uleutf_codepoint(const char* str, ule_i32* out_codepoint);

/* Calculates the size of the next utf8 codepoint in str */
size_t uleutf_codepointcalcsize(const char* str);

/* Returns the size of the given codepoint in bytes */
size_t uleutf_codepointsize(ule_i32 chr);

/* Returns the number of terminal columns the given codepoint occupies when
   printed with a fixed-width font: 0 for zero-width codepoints (the null
   codepoint and combining marks), 2 for wide (double-width) codepoints, 1 for
   ordinary codepoints, and -1 for non-printable control codepoints */
int uleutf_codepointvisualwidth(ule_i32 chr);

/* Make a codepoint lower case if possible */
ule_i32 uleutf_lwrcodepoint(ule_i32 cp);

/* Make a codepoint upper case if possible */
ule_i32 uleutf_uprcodepoint(ule_i32 cp);

/* TODO */
char* uleutf_cpy(char* dst, const char* src);
size_t uleutf_cspn(const char* src, const char* reject);
char* uleutf_dup(const char* src);
size_t uleutf_len(const char* str);
size_t uleutf_nlen(const char* str, size_t n);
int uleutf_ncasecmp(const char* src1, const char* src2, size_t n);
char* uleutf_ncat(char* dst, const char* src, size_t n);
char* uleutf_ncpy(char* dst, const char* src, size_t n);
char* uleutf_ndup(const char* src, size_t n);
char* uleutf_pbrk(const char* str, const char* accept);
char* uleutf_rchr(const char* src, ule_i32 chr);
size_t uleutf_size(const char* str);
size_t uleutf_size_lazy(const char* str);
size_t uleutf_nsize_lazy(const char* str, size_t n);
size_t uleutf_spn(const char* src, const char* accept);
char* uleutf_casestr(const char* haystack, const char* needle);
int uleutf_makevalid(char* str, ule_i32 replacement);
char* uleutf_catcodepoint(char* str, ule_i32 chr, size_t n);
int uleutf_islower(ule_i32 chr);
int uleutf_isupper(ule_i32 chr);
void uleutf_lwr(char* str);
void uleutf_upr(char* str);
char* uleutf_rcodepoint(const char* str, ule_i32* out_codepoint);
char* uleutf_dup_ex(const char* src, char* (*alloc_func_ptr)(char*, size_t), char* user_data);
char* uleutf_ndup_ex(const char* src, size_t n, char* (*alloc_func_ptr)(char*, size_t), char* user_data);

#endif /* ULEUTF_H */

