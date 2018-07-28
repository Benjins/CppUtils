#ifndef MACROS_H
#define MACROS_H

#pragma once

#define BNS_ARRAY_COUNT(x) ((int)(sizeof(x)/sizeof((x)[0])))

#define BNS_UNUSED(x) (void(x))

#define BNS_MIN(a,b) ((a) < (b) ? (a) : (b))
#define BNS_MAX(a,b) ((a) > (b) ? (a) : (b))

inline constexpr float bns_abs(float x) { return (x < 0) ? -x : x; }
inline constexpr int bns_abs(int x) { return (x < 0) ? -x : x; }

#define BNS_ABS(x) bns_abs(x)
#define BNS_SQR(x) ((x)*(x))

#define BNS_MEMCPY(dst, src, size) for(int __i = 0; __i < (int)(size); __i++){((char*)dst)[__i] = ((const char*)src)[__i];}

#define BNS_OFFSET_OF(type, field) (int)(size_t)(&((type*)0)->field)

#define BNS_GLUE_TOKS_(a, b) a ## b
#define BNS_GLUE_TOKS(a, b) BNS_GLUE_TOKS_(a, b)

#define BNS_STRINGIFY_NOX(x) #x
#define BNS_STRINGIFY(x) BNS_STRINGIFY_NOX(x)

#define BNS_DO_EXACTLY_ONCE() static int BNS_GLUE_TOKS(_only_once_, __LINE__) = 1; \
							if ( BNS_GLUE_TOKS(_only_once_, __LINE__) && !( BNS_GLUE_TOKS(_only_once_, __LINE__) = 0))

#define BNS_PI 3.1415926535897932384f
#define BNS_DEG2RAD (BNS_PI / 180.0f)
#define BNS_RAD2DEG (180.0f / BNS_PI)

#endif

