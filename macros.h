#ifndef MACROS_H
#define MACROS_H

#pragma once

#define BNS_ARRAY_COUNT(x) ((int)(sizeof(x)/sizeof((x)[0])))

#define BNS_UNUSED(x) (void(x))

#define BNS_MIN(a,b) ((a) < (b) ? (a) : (b))
#define BNS_MAX(a,b) ((a) > (b) ? (a) : (b))
#define BNS_CLAMP(val, min, max) (BNS_MAX((min), BNS_MIN((max), (val))))

static_assert(BNS_CLAMP(1, 2, 5) == 2, "Check clamp");
static_assert(BNS_CLAMP(2, 2, 5) == 2, "Check clamp");
static_assert(BNS_CLAMP(3, 2, 5) == 3, "Check clamp");
static_assert(BNS_CLAMP(5, 2, 5) == 5, "Check clamp");
static_assert(BNS_CLAMP(7, 2, 5) == 5, "Check clamp");

inline constexpr double bns_abs(double x) { return (x < 0) ? -x : x; }
inline constexpr float bns_abs(float x) { return (x < 0) ? -x : x; }
inline constexpr int bns_abs(int x) { return (x < 0) ? -x : x; }

#define BNS_ABS(x) bns_abs(x)
#define BNS_SQR(x) ((x)*(x))

#define BNS_ROUND(x) ((int)((x) + ((x) < 0 ? -0.5f : 0.5f)))

static_assert(BNS_ROUND(0.0f) == 0, "Check round");
static_assert(BNS_ROUND(0.1f) == 0, "Check round");
static_assert(BNS_ROUND(0.3f) == 0, "Check round");
static_assert(BNS_ROUND(0.6f) == 1, "Check round");
static_assert(BNS_ROUND(1.6f) == 2, "Check round");
static_assert(BNS_ROUND(-0.1f) == 0, "Check round");
static_assert(BNS_ROUND(-0.6f) == -1, "Check round");
static_assert(BNS_ROUND(-1.6f) == -2, "Check round");

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

#define BNS_SWAP_VAL(a, b) do { auto _bns_tmp = a; a = b; b = _bns_tmp; } while(0)

#endif

