#pragma region Copyright (c) 2014-2016 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#undef M_PI

#ifdef _MSC_VER
#include <time.h>
#endif

#include <fat.h>

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>

#include "diagnostic.h"
#include <gccore.h>
#include <ogc/lwp_watchdog.h>

typedef int8_t sint8;
typedef int16_t sint16;
typedef int32_t sint32;
typedef int64_t sint64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef char utf8;
typedef utf8* utf8string;
typedef const utf8* const_utf8string;
#ifdef __WINDOWS__
typedef wchar_t utf16;
typedef utf16* utf16string;
#endif

typedef uint32 codepoint_t;
typedef uint8 colour_t;

#define rol8(x, shift)		(((uint8)(x) << (shift)) | ((uint8)(x) >> (8 - (shift))))
#define ror8(x, shift)		(((uint8)(x) >> (shift)) | ((uint8)(x) << (8 - (shift))))
#define rol16(x, shift)		(((uint16)(x) << (shift)) | ((uint16)(x) >> (16 - (shift))))
#define ror16(x, shift)		(((uint16)(x) >> (shift)) | ((uint16)(x) << (16 - (shift))))
#define rol32(x, shift)		(((uint32)(x) << (shift)) | ((uint32)(x) >> (32 - (shift))))
#define ror32(x, shift)		(((uint32)(x) >> (shift)) | ((uint32)(x) << (32 - (shift))))
#define rol64(x, shift)		(((uint64)(x) << (shift)) | ((uint32)(x) >> (64 - (shift))))
#define ror64(x, shift)		(((uint64)(x) >> (shift)) | ((uint32)(x) << (64 - (shift))))

#ifndef __cplusplus
// in C++ you should be using Math::Min and Math::Max
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#define sgn(x)				((x > 0) ? 1 : ((x < 0) ? -1 : 0))
#define clamp(l, x, h)		(min(h, max(l, x)))

#endif // __cplusplus

// Rounds an integer down to the given power of 2. y must be a power of 2.
#define floor2(x, y)		((x) & (~((y) - 1)))

// Rounds an integer up to the given power of 2. y must be a power of 2.
#define ceil2(x, y)			(((x) + (y) - 1) & (~((y) - 1)))


#ifndef __cplusplus
// in C++ you should be using Util::CountOf
#ifdef __GNUC__
/**
 * Force a compilation error if condition is true, but also produce a
 * result (of value 0 and type size_t), so the expression can be used
 * e.g. in a structure initializer (or where-ever else comma expressions
 * aren't permitted).
 */
#define BUILD_BUG_ON_ZERO(e) (sizeof(struct { int:-!!(e); }))

/* &a[0] degrades to a pointer: a different type from an array */
#define __must_be_array(a) \
        BUILD_BUG_ON_ZERO(__builtin_types_compatible_p(typeof(a), typeof(&a[0])))

// based on http://lxr.free-electrons.com/source/include/linux/kernel.h#L54
#define countof(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))
#elif defined (_MSC_VER)
#define countof(arr)			_countof(arr)
#else
#define countof(arr)			(sizeof(arr) / sizeof((arr)[0]))
#endif // __GNUC__
#endif // __cplusplus

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)) || defined(__NDS__) || defined(__WII__)
#include <unistd.h>
#define STUB() log_warning("Function %s at %s:%d is a stub.\n", __PRETTY_FUNCTION__, __FILE__, __LINE__)
#define _strcmpi _stricmp
#define _stricmp(x, y) strcasecmp((x), (y))
#define _strnicmp(x, y, n) strncasecmp((x), (y), (n))
#define _strdup(x) strdup((x))

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define RCT2_ENDIANESS __ORDER_LITTLE_ENDIAN__
#define LOBYTE(w) ((uint8)(w))
#define HIBYTE(w) ((uint8)(((uint16)(w)>>8)&0xFF))
#endif // __BYTE_ORDER__

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define RCT2_ENDIANESS __ORDER_BIG_ENDIAN__
//ehm, don't know about this :S
#define LOBYTE(w) ((uint8)(w))
#define HIBYTE(w) ((uint8)(((uint16)(w)>>8)&0xFF))
#endif // __BYTE_ORDER__

#ifndef RCT2_ENDIANESS
#error Unknown endianess!
#endif // RCT2_ENDIANESS

#endif // defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))

#if !((defined (_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L) || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 700))
char *strndup(const char *src, size_t size);
#endif // !(POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700)

// BSD and macOS have MAP_ANON instead of MAP_ANONYMOUS
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#include "version.h"

#define OPENRCT2_MASTER_SERVER_URL	"https://servers.openrct2.website"

// Time (represented as number of 100-nanosecond intervals since 0001-01-01T00:00:00Z)
typedef uint64 datetime64;

#define DATETIME64_MIN ((datetime64)0)

// Represent fixed point numbers. dp = decimal point
typedef uint8 fixed8_1dp;
typedef uint8 fixed8_2dp;
typedef sint16 fixed16_1dp;
typedef sint16 fixed16_2dp;
typedef sint32 fixed32_1dp;
typedef sint32 fixed32_2dp;

// Money is stored as a multiple of 0.10.
typedef fixed8_1dp money8;
typedef fixed16_1dp money16;
typedef fixed32_1dp money32;

// Construct a fixed point number. For example, to create the value 3.65 you
// would write FIXED_2DP(3,65)
#define FIXED_XDP(x, whole, fraction)	((whole) * (10 * x) + (fraction))
#define FIXED_1DP(whole, fraction)		FIXED_XDP(1, whole, fraction)
#define FIXED_2DP(whole, fraction)		FIXED_XDP(10, whole, fraction)

// Construct a money value in the format MONEY(10,70) to represent 10.70. Fractional part must be two digits.
#define MONEY(whole, fraction)			((whole) * 10 + ((fraction) / 10))

#define MONEY_FREE						MONEY(0,00)
#define MONEY32_UNDEFINED				((money32)0x80000000)

typedef void (EMPTY_ARGS_VOID_POINTER)();
typedef unsigned short rct_string_id;

#define SafeFree(x) do { free(x); (x) = NULL; } while (0)

#define SafeDelete(x) do { delete (x); (x) = nullptr; } while (0)
#define SafeDeleteArray(x) do { delete[] (x); (x) = nullptr; } while (0)

#ifndef interface
	#define interface struct
#endif
#define abstract = 0

#if defined(__i386__) || defined(_M_IX86)
#define PLATFORM_X86
#else
#define NO_RCT2 1
#endif

#if defined(__LP64__) || defined(_WIN64)
	#define PLATFORM_64BIT
#else
	#define PLATFORM_32BIT
#endif

// C99's restrict keywords guarantees the pointer in question, for the whole of its lifetime,
// will be the only way to access a given memory region. In other words: there is no other pointer
// aliasing the same memory area. Using it lets compiler generate better code. If your compiler
// does not support it, feel free to drop it, at some performance hit.
#ifdef __cplusplus
	#ifdef _MSC_VER
		#define RESTRICT __restrict
	#else
		#define RESTRICT __restrict__
	#endif
#else
	#ifdef _MSC_VER
		#define RESTRICT __restrict
	#else
		#define RESTRICT restrict
	#endif
#endif
#ifndef RESTRICT
	#define RESTRICT
#endif

#ifdef __cplusplus
#define assert_struct_size(x, y) //_Static_assert(sizeof(x) == (y), "Improper struct size")
#else
	// Visual Studio does not know _Static_assert
	#if !defined(_MSC_VER)
		#define assert_struct_size(x, y) _Static_assert(sizeof(x) == (y), "Improper struct size")
	#else
		#define assert_struct_size(x, y)
	#endif // !defined(_MSC_VER)
#endif

#ifdef PLATFORM_X86
	#ifndef FASTCALL
		#ifdef __GNUC__
			#define FASTCALL __attribute__((fastcall))
		#elif defined(_MSC_VER)
			#define FASTCALL __fastcall
		#else
			#pragma message "Not using fastcall calling convention, please check your compiler support"
			#define FASTCALL
		#endif
	#endif // FASTCALL
#else // PLATFORM_X86
	#define FASTCALL
#endif // PLATFORM_X86

/**
 * x86 register structure, only used for easy interop to RCT2 code.
 */
#pragma pack(push, 1)
typedef struct registers {
	union {
		int eax;
		struct 
		{
			short : 16;
			short ax;
		};
		struct {
			short : 16;
			char ah;
			char al;
		};
	};
	union {
		int ebx;
		struct 
		{
			short : 16;
			short bx;
		};
		struct {
			short : 16;
			char bh;
			char bl;
		};
	};
	union {
		int ecx;
		struct 
		{
			short : 16;
			short cx;
		};
		struct {
			short : 16;
			char ch;
			char cl;
		};
	};
	union {
		int edx;
		struct 
		{
			short : 16;
			short dx;
		};
		struct {
			short : 16;
			char dh;
			char dl;
		};
	};
	union {
		int esi;
		struct 
		{
			short : 16;
			short si;
		};
	};
	union {
		int edi;
		struct 
		{
			short : 16;
			short di;
		};
	};
	union {
		int ebp;
		struct 
		{
			short : 16;
			short bp;
		};
	};
} registers;
assert_struct_size(registers, 7 * 4);
#pragma pack(pop)

#define UNUSED(x) ((void)(x))

#define RW_SEEK_SET		SEEK_SET
#define RW_SEEK_CUR		SEEK_CUR
#define RW_SEEK_END		SEEK_END

typedef FILE SDL_RWops;
#define SDL_RWFromFile(path, mode) fopen((path), (mode))
#define SDL_RWseek(handle, count, base) fseek((handle), (count), (base))
#define SDL_RWread(handle, buf, each, nr) fread((buf), (each), (nr), (handle))
#define SDL_RWwrite(handle, buf, each, nr) fwrite((buf), (each), (nr), (handle))
#define SDL_RWclose(handle) fclose(handle)
#define SDL_RWtell(handle) ftell(handle)
#define SDL_GetError() 0

#define SDL_RWFromMem(buf, size) fmemopen((buf), (size), "rb")

static inline uint64_t SDL_RWsize(SDL_RWops* file)
{
	uint32_t curpos = (uint32_t)ftell(file);
	fseek(file, 0, SEEK_END);
    uint32_t len = (uint32_t)ftell(file);
	fseek(file, curpos, SEEK_SET);
	return len;
}

typedef struct SDL_Color {
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 unused;
} SDL_Color;
#define SDL_Colour SDL_Color

#define SDL_GetTicks() ticks_to_millisecs(SYS_Time())
#define SDL_Delay(time) usleep((time) * 1000)

#define SDL_GetPerformanceFrequency() secs_to_ticks(1)
#define SDL_GetPerformanceCounter() SYS_Time()

#define SDL_MIX_MAXVOLUME 128

typedef int8_t Sint8;
typedef int16_t Sint16;
typedef int32_t Sint32;
typedef int64_t Sint64;
typedef uint8_t Uint8;
typedef uint16_t Uint16;
typedef uint32_t Uint32;
typedef uint64_t Uint64;

/* The two types of endianness */
#define SDL_LIL_ENDIAN	1234
#define SDL_BIG_ENDIAN	4321

#define SDL_BYTEORDER	SDL_BIG_ENDIAN

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* Use inline functions for compilers that support them, and static
   functions for those that do not.  Because these functions become
   static for compilers that do not support inline functions, this
   header should only be included in files that actually use them.
*/
#if defined(__GNUC__) && defined(__i386__) && \
   !(__GNUC__ == 2 && __GNUC_MINOR__ <= 95 /* broken gcc version */)
static __inline__ uint16 SDL_Swap16(uint16 x)
{
	__asm__("xchgb %b0,%h0" : "=q" (x) :  "0" (x));
	return x;
}
#elif defined(__GNUC__) && defined(__x86_64__)
static __inline__ uint16 SDL_Swap16(uint16 x)
{
	__asm__("xchgb %b0,%h0" : "=Q" (x) :  "0" (x));
	return x;
}
#elif defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))
static __inline__ uint16 SDL_Swap16(uint16 x)
{
	uint16 result;

	__asm__("rlwimi %0,%2,8,16,23" : "=&r" (result) : "0" (x >> 8), "r" (x));
	return result;
}
#elif defined(__GNUC__) && (defined(__M68000__) || defined(__M68020__))
static __inline__ uint16 SDL_Swap16(uint16 x)
{
	__asm__("rorw #8,%0" : "=d" (x) :  "0" (x) : "cc");
	return x;
}
#else
static __inline__ uint16 SDL_Swap16(uint16 x) {
	return((x<<8)|(x>>8));
}
#endif

#if defined(__GNUC__) && defined(__i386__) && \
   !(__GNUC__ == 2 && __GNUC_MINOR__ <= 95 /* broken gcc version */)
static __inline__ uint32 SDL_Swap32(uint32 x)
{
	__asm__("bswap %0" : "=r" (x) : "0" (x));
	return x;
}
#elif defined(__GNUC__) && defined(__x86_64__)
static __inline__ uint32 SDL_Swap32(uint32 x)
{
	__asm__("bswapl %0" : "=r" (x) : "0" (x));
	return x;
}
#elif defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))
static __inline__ uint32 SDL_Swap32(uint32 x)
{
	uint32 result;

	__asm__("rlwimi %0,%2,24,16,23" : "=&r" (result) : "0" (x>>24), "r" (x));
	__asm__("rlwimi %0,%2,8,8,15"   : "=&r" (result) : "0" (result),    "r" (x));
	__asm__("rlwimi %0,%2,24,0,7"   : "=&r" (result) : "0" (result),    "r" (x));
	return result;
}
#elif defined(__GNUC__) && (defined(__M68000__) || defined(__M68020__))
static __inline__ uint32 SDL_Swap32(uint32 x)
{
	__asm__("rorw #8,%0\n\tswap %0\n\trorw #8,%0" : "=d" (x) :  "0" (x) : "cc");
	return x;
}
#else
static __inline__ uint32 SDL_Swap32(uint32 x) {
	return((x<<24)|((x<<8)&0x00FF0000)|((x>>8)&0x0000FF00)|(x>>24));
}
#endif

#ifdef SDL_HAS_64BIT_TYPE
#if defined(__GNUC__) && defined(__i386__) && \
   !(__GNUC__ == 2 && __GNUC_MINOR__ <= 95 /* broken gcc version */)
static __inline__ uint64 SDL_Swap64(uint64 x)
{
	union { 
		struct { uint32 a,b; } s;
		uint64 u;
	} v;
	v.u = x;
	__asm__("bswapl %0 ; bswapl %1 ; xchgl %0,%1" 
	        : "=r" (v.s.a), "=r" (v.s.b) 
	        : "0" (v.s.a), "1" (v.s.b)); 
	return v.u;
}
#elif defined(__GNUC__) && defined(__x86_64__)
static __inline__ uint64 SDL_Swap64(Uint64 x)
{
	__asm__("bswapq %0" : "=r" (x) : "0" (x));
	return x;
}
#else
static __inline__ uint64 SDL_Swap64(Uint64 x)
{
	uint32 hi, lo;

	/* Separate into high and low 32-bit values and swap them */
	lo = (Uint32)(x&0xFFFFFFFF);
	x >>= 32;
	hi = (Uint32)(x&0xFFFFFFFF);
	x = SDL_Swap32(lo);
	x <<= 32;
	x |= SDL_Swap32(hi);
	return(x);
}
#endif
#else
/* This is mainly to keep compilers from complaining in SDL code.
   If there is no real 64-bit datatype, then compilers will complain about
   the fake 64-bit datatype that SDL provides when it compiles user code.
*/
#define SDL_Swap64(X)	(X)
#endif /* SDL_HAS_64BIT_TYPE */


/* Byteswap item from the specified endianness to the native endianness */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define SDL_SwapLE16(X)	(X)
#define SDL_SwapLE32(X)	(X)
#define SDL_SwapLE64(X)	(X)
#define SDL_SwapBE16(X)	SDL_Swap16(X)
#define SDL_SwapBE32(X)	SDL_Swap32(X)
#define SDL_SwapBE64(X)	SDL_Swap64(X)
#else
#define SDL_SwapLE16(X)	SDL_Swap16(X)
#define SDL_SwapLE32(X)	SDL_Swap32(X)
#define SDL_SwapLE64(X)	SDL_Swap64(X)
#define SDL_SwapBE16(X)	(X)
#define SDL_SwapBE32(X)	(X)
#define SDL_SwapBE64(X)	(X)
#endif

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

/* Read an item of the specified endianness and return in native format */
static inline Uint8 SDL_ReadU8(SDL_RWops *src)
{
	Uint8 result;
	fread(&result, 1, 1, src);
	return result;
}
static inline Uint16 SDL_ReadLE16(SDL_RWops *src)
{
	Uint16 result;
	fread(&result, 2, 1, src);
	return SDL_SwapLE16(result);
}
static inline Uint16 SDL_ReadBE16(SDL_RWops *src)
{
	Uint32 result;
	fread(&result, 2, 1, src);
	return SDL_SwapBE16(result);
}
static inline Uint32 SDL_ReadLE32(SDL_RWops *src)
{
	Uint32 result;
	fread(&result, 4, 1, src);
	return SDL_SwapLE32(result);
}
static inline Uint32 SDL_ReadBE32(SDL_RWops *src)
{
	Uint32 result;
	fread(&result, 4, 1, src);
	return SDL_SwapBE32(result);
}
static inline Uint64 SDL_ReadLE64(SDL_RWops *src)
{
	Uint64 result;
	fread(&result, 8, 1, src);
	return SDL_SwapLE64(result);
}
static inline Uint64 SDL_ReadBE64(SDL_RWops *src)
{
	Uint64 result;
	fread(&result, 8, 1, src);
	return SDL_SwapBE64(result);
}

/* Write an item of native format to the specified endianness */
static inline int SDL_WriteU8(SDL_RWops *dst, Uint8 value)
{
	return fwrite(&value, 1, 1, dst);
}
static inline int SDL_WriteLE16(SDL_RWops *dst, Uint16 value)
{
	value = SDL_SwapLE16(value);
	return fwrite(&value, 2, 1, dst);
}
static inline int SDL_WriteBE16(SDL_RWops *dst, Uint16 value)
{
	value = SDL_SwapBE16(value);
	return fwrite(&value, 2, 1, dst);
}
static inline int SDL_WriteLE32(SDL_RWops *dst, Uint32 value)
{
	value = SDL_SwapLE32(value);
	return fwrite(&value, 4, 1, dst);
}
static inline int SDL_WriteBE32(SDL_RWops *dst, Uint32 value)
{
	value = SDL_SwapBE32(value);
	return fwrite(&value, 4, 1, dst);
}
static inline int SDL_WriteLE64(SDL_RWops *dst, Uint64 value)
{
	value = SDL_SwapLE64(value);
	return fwrite(&value, 8, 1, dst);
}
static inline int SDL_WriteBE64(SDL_RWops *dst, Uint64 value)
{
	value = SDL_SwapBE64(value);
	return fwrite(&value, 8, 1, dst);
}

/**
 *  \brief Audio format flags.
 *  
 *  These are what the 16 bits in SDL_AudioFormat currently mean...
 *  (Unspecified bits are always zero).
 *  
 *  \verbatim
    ++-----------------------sample is signed if set
    ||
    ||       ++-----------sample is bigendian if set
    ||       ||
    ||       ||          ++---sample is float if set
    ||       ||          ||
    ||       ||          || +---sample bit size---+
    ||       ||          || |                     |
    15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    \endverbatim
 *  
 *  There are macros in SDL 2.0 and later to query these bits.
 */
typedef Uint16 SDL_AudioFormat;

/**
 *  \name Audio flags
 */
/*@{*/

#define SDL_AUDIO_MASK_BITSIZE       (0xFF)
#define SDL_AUDIO_MASK_DATATYPE      (1<<8)
#define SDL_AUDIO_MASK_ENDIAN        (1<<12)
#define SDL_AUDIO_MASK_SIGNED        (1<<15)
#define SDL_AUDIO_BITSIZE(x)         (x & SDL_AUDIO_MASK_BITSIZE)
#define SDL_AUDIO_ISFLOAT(x)         (x & SDL_AUDIO_MASK_DATATYPE)
#define SDL_AUDIO_ISBIGENDIAN(x)     (x & SDL_AUDIO_MASK_ENDIAN)
#define SDL_AUDIO_ISSIGNED(x)        (x & SDL_AUDIO_MASK_SIGNED)
#define SDL_AUDIO_ISINT(x)           (!SDL_AUDIO_ISFLOAT(x))
#define SDL_AUDIO_ISLITTLEENDIAN(x)  (!SDL_AUDIO_ISBIGENDIAN(x))
#define SDL_AUDIO_ISUNSIGNED(x)      (!SDL_AUDIO_ISSIGNED(x))

/** 
 *  \name Audio format flags
 *
 *  Defaults to LSB byte order.
 */
/*@{*/
#define AUDIO_U8	0x0008  /**< Unsigned 8-bit samples */
#define AUDIO_S8	0x8008  /**< Signed 8-bit samples */
#define AUDIO_U16LSB	0x0010  /**< Unsigned 16-bit samples */
#define AUDIO_S16LSB	0x8010  /**< Signed 16-bit samples */
#define AUDIO_U16MSB	0x1010  /**< As above, but big-endian byte order */
#define AUDIO_S16MSB	0x9010  /**< As above, but big-endian byte order */
#define AUDIO_U16	AUDIO_U16LSB
#define AUDIO_S16	AUDIO_S16LSB
/*@}*/

/**
 *  \name int32 support
 *  
 *  New to SDL 1.3.
 */
/*@{*/
#define AUDIO_S32LSB	0x8020  /**< 32-bit integer samples */
#define AUDIO_S32MSB	0x9020  /**< As above, but big-endian byte order */
#define AUDIO_S32	AUDIO_S32LSB
/*@}*/

/**
 *  \name float32 support
 *  
 *  New to SDL 1.3.
 */
/*@{*/
#define AUDIO_F32LSB	0x8120  /**< 32-bit floating point samples */
#define AUDIO_F32MSB	0x9120  /**< As above, but big-endian byte order */
#define AUDIO_F32	AUDIO_F32LSB
/*@}*/

/**
 *  \name Native audio byte ordering
 */
/*@{*/
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define AUDIO_U16SYS	AUDIO_U16LSB
#define AUDIO_S16SYS	AUDIO_S16LSB
#define AUDIO_S32SYS	AUDIO_S32LSB
#define AUDIO_F32SYS	AUDIO_F32LSB
#else
#define AUDIO_U16SYS	AUDIO_U16MSB
#define AUDIO_S16SYS	AUDIO_S16MSB
#define AUDIO_S32SYS	AUDIO_S32MSB
#define AUDIO_F32SYS	AUDIO_F32MSB
#endif
/*@}*/

/** 
 *  \name Allow change flags
 *  
 *  Which audio format changes are allowed when opening a device.
 */
/*@{*/
#define SDL_AUDIO_ALLOW_FREQUENCY_CHANGE    0x00000001
#define SDL_AUDIO_ALLOW_FORMAT_CHANGE       0x00000002
#define SDL_AUDIO_ALLOW_CHANNELS_CHANGE     0x00000004
#define SDL_AUDIO_ALLOW_ANY_CHANGE          (SDL_AUDIO_ALLOW_FREQUENCY_CHANGE|SDL_AUDIO_ALLOW_FORMAT_CHANGE|SDL_AUDIO_ALLOW_CHANNELS_CHANGE)
/*@}*/

/*@}*//*Audio flags*/ 


#ifdef __cplusplus
extern "C" {
#endif
	#include "../platform/platform.h"
#ifdef __cplusplus
}
#endif

#endif
