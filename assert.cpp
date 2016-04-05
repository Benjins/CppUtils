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

void assertFrom(const char* cond, const char* function, const char* file, int line, const char* msg){
	printf("\nASSERTION!\n");
	if (cond != nullptr){
		printf("Condition: %s\n", cond);
	}

	if(msg != nullptr){
		printf("Message: %s\n", msg);
	}

	printf("File: %s\nLine: %d\nFunction: %s\n", file, line, function);

#if defined(EXIT_ON_ASSERT)
	exit(-1);
#else

	printf("Type b to break, or s to skip.\n");

	while(true){
		char response[256] = {0};
		fgets(response, sizeof(response) - 1, stdin);

		if(response[0] == 'b'){
			DEBUG_BREAK();
			break;
		}
		else if(response[0] == 's'){
			break;
		}
		else{
			printf("Please enter b or s.\n");
		}
	}
#endif
}

#endif


#if ASSERT_TEST_MAIN

int assertCount = 0;

void CheckAssertCount(int expected){
	if(assertCount != expected){
		printf("Expected assertions: %d,  actually: %d\n", expected, assertCount);
		exit(-1);
	}
}

void TrapHandler(int sigNum){
	assertCount++;
	BNS_UNUSED(sigNum);
}

int main(){
	signal(SIGTRAP, TrapHandler);

	CheckAssertCount(0);
	ASSERT(1 == 1);
	CheckAssertCount(0);
	ASSERT(1 == 3);
	CheckAssertCount(1);
	ASSERT_MSG(1 == 5, "Hi.%s\n", "Bye.");
	CheckAssertCount(2);
	ASSERT_WARN("%s", "This is a warning.");
	CheckAssertCount(3);
	ASSERT_MSG(1 == 1, "%s", "Should noy go off.");
	CheckAssertCount(3);

	return 0;
}

#endif


#endif
