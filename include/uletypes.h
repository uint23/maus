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

/* Guidance:
   If you want to use the shorthand type names (eg u8 instead of ule_u8), you
   must define ULE_SHORT_TYPE_NAMES before including this header.

   If you want to define your own pointer types, you must define ULE_UPTR_TYPE
   and ULE_IPTR_TYPE before including this header. */

#ifndef ULETYPES_H
#define ULETYPES_H

#include <limits.h>
#include <stddef.h>

#if CHAR_BIT != 8
	#error "libule: 8-bit bytes are required"
#endif

/* 8-bit */
typedef unsigned char ule_u8;
typedef signed char   ule_i8;

/* 16-bit */
#if USHRT_MAX == 0xFFFFU
	typedef unsigned short ule_u16;
	typedef signed short   ule_i16;
	#define ULE_I16_MAX ((ule_i16)SHRT_MAX)
	#define ULE_I16_MIN ((ule_i16)SHRT_MIN)
#elif UINT_MAX == 0xFFFFU
	typedef unsigned int ule_u16;
	typedef signed int   ule_i16;
	#define ULE_I16_MAX ((ule_i16)INT_MAX)
	#define ULE_I16_MIN ((ule_i16)INT_MIN)
#else
	#error "libule: no 16-bit integer type"
#endif

/* 32-bit */
#if UINT_MAX == 0xFFFFFFFFUL
	typedef unsigned int ule_u32;
	typedef signed int   ule_i32;
	#define ULE_I32_MAX ((ule_i32)INT_MAX)
	#define ULE_I32_MIN ((ule_i32)INT_MIN)
#elif ULONG_MAX == 0xFFFFFFFFUL
	typedef unsigned long ule_u32;
	typedef signed long   ule_i32;
	#define ULE_I32_MAX ((ule_i32)LONG_MAX)
	#define ULE_I32_MIN ((ule_i32)LONG_MIN)
#elif USHRT_MAX == 0xFFFFFFFFUL
	typedef unsigned short ule_u32;
	typedef signed short   ule_i32;
	#define ULE_I32_MAX ((ule_i32)SHRT_MAX)
	#define ULE_I32_MIN ((ule_i32)SHRT_MIN)
#else
	#error "libule: no 32-bit integer type"
#endif

/* 64-bit */
#if ULONG_MAX > 0xFFFFFFFFUL
	typedef unsigned long ule_u64;
	typedef signed long   ule_i64;
	#define ULE_I64_MAX ((ule_i64)LONG_MAX)
	#define ULE_I64_MIN ((ule_i64)LONG_MIN)
	#define ULE_HAS_64 1
#elif defined(_WIN64)
	typedef size_t    ule_u64;
	typedef ptrdiff_t ule_i64;
	#define ULE_HAS_64 1
	#define ULE_I64_TWOS_COMPLEMENT 1
#endif

typedef char ule_check_u8[(sizeof(ule_u8) * CHAR_BIT == 8) ? 1 : -1];
typedef char ule_check_i8[(sizeof(ule_i8) * CHAR_BIT == 8) ? 1 : -1];
typedef char ule_check_u16[(sizeof(ule_u16) * CHAR_BIT == 16) ? 1 : -1];
typedef char ule_check_i16[(sizeof(ule_i16) * CHAR_BIT == 16) ? 1 : -1];
typedef char ule_check_u32[(sizeof(ule_u32) * CHAR_BIT == 32) ? 1 : -1];
typedef char ule_check_i32[(sizeof(ule_i32) * CHAR_BIT == 32) ? 1 : -1];

#ifdef ULE_HAS_64
	typedef char ule_check_u64[(sizeof(ule_u64) * CHAR_BIT == 64) ? 1 : -1];
	typedef char ule_check_i64[(sizeof(ule_i64) * CHAR_BIT == 64) ? 1 : -1];
#endif /* ULE_HAS_64 */

/* Pointer Length Sizes */
#if defined(ULE_UPTR_TYPE) != defined(ULE_IPTR_TYPE)
	#error "libule: define both ULE_UPTR_TYPE and ULE_IPTR_TYPE"
#endif

#if defined(ULE_UPTR_TYPE)
	typedef ULE_UPTR_TYPE ule_uptr;
	typedef ULE_IPTR_TYPE ule_iptr;

#elif defined(_WIN64) || defined(_WIN32) || \
      defined(__LP64__) || defined(_LP64) || \
      defined(__ILP32__) || defined(_ILP32)
	typedef size_t    ule_uptr;
	typedef ptrdiff_t ule_iptr;

#else
	#error "libule: define ULE_UPTR_TYPE and ULE_IPTR_TYPE for this target"
#endif

typedef char ule_check_uptr[(sizeof(ule_uptr) == sizeof(void*)) ? 1 : -1];
typedef char ule_check_iptr[(sizeof(ule_iptr) == sizeof(void*)) ? 1 : -1];

/* Limits */
#define ULE_U8_MAX ((ule_u8)0xFFU)
#define ULE_I8_MAX ((ule_i8)SCHAR_MAX)
#define ULE_I8_MIN ((ule_i8)SCHAR_MIN)

#define ULE_U16_MAX ((ule_u16)0xFFFFU)
#define ULE_U32_MAX ((ule_u32)0xFFFFFFFFUL)

#ifdef ULE_HAS_64
	#define ULE_U64_MAX ((ule_u64)-1)

	#ifdef ULE_I64_TWOS_COMPLEMENT
		#define ULE_I64_MAX ((ule_i64)(ULE_U64_MAX >> 1))
		#define ULE_I64_MIN ((ule_i64)(-ULE_I64_MAX - 1))
	#endif /* ULE_I64_TWOS_COMPLEMENT */
#endif /* ULE_HAS_64 */

/* Size types */
typedef size_t    ule_usize;
typedef ptrdiff_t ule_isize;

/* (Optional) Short names */
#ifdef ULE_SHORT_TYPE_NAMES
	typedef ule_u8  u8;
	typedef ule_i8  i8;
	typedef ule_u16 u16;
	typedef ule_i16 i16;
	typedef ule_u32 u32;
	typedef ule_i32 i32;

	#ifdef ULE_HAS_64
		typedef ule_u64 u64;
		typedef ule_i64 i64;
	#endif /* ULE_HAS_64 */

	typedef ule_uptr  uptr;
	typedef ule_iptr  iptr;
	typedef ule_usize usize;
	typedef ule_isize isize;
#endif /* ULE_SHORT_TYPE_NAMES */

#endif /* ULETYPES_H */

