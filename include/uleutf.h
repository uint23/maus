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

char* uleutf_codepoint(const char* str, ule_i32* out_codepoint);
size_t uleutf_codepointcalcsize(const char* str);
size_t uleutf_codepointsize(ule_i32 chr);
int uleutf_codepointvisualwidth(ule_i32 chr);
size_t uleutf_len(const char* str);
size_t uleutf_nlen(const char* str, size_t n);

#endif /* ULEUTF_H */
