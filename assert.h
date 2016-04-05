#ifndef ASSERT_H
#define ASSERT_H

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "macros.h"

#define STRINGIFY(x) #x

#if defined(BNS_DEBUG)
#define ASSERT(cond) if (!(cond)){\
	assertFrom(#cond, __FUNCTION__, __FILE__, __LINE__, nullptr);\
}
#define ASSERT_MSG(cond, str, ...) if(!(cond)){\
	char assertMsg[4096];\
	snprintf(assertMsg, 4096, str, __VA_ARGS__);\
	assertFrom(#cond, __FUNCTION__, __FILE__, __LINE__, assertMsg);\
}
#define ASSERT_WARN(str, ...) {\
	char assertMsg[4096];\
	snprintf(assertMsg, 4096, str, __VA_ARGS__);\
	assertFrom(nullptr, __FUNCTION__, __FILE__, __LINE__, assertMsg);\
}

#else

#define ASSERT(cond)
#define ASSERT_MSG(cond, str, ...)
#define ASSERT_WARN(str, ...)

#endif

#if defined(BNS_DEBUG)

#if defined(_MSC_VER)
#define DEBUG_BREAK() __debugbreak()
#else

#include <signal.h>

#if defined(_WIN32) || defined(_WIN64)
#define DEBUG_BREAK() __builtin_trap()
#else
#define DEBUG_BREAK() raise(SIGTRAP);
#endif

#endif

void assertFrom(const char* cond, const char* function, const char* file, int line, const char* msg);

#endif


#endif
