#include "assert.h"

#if defined(_MSC_VER)
#include <Windows.h>
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

#if defined(_MSC_VER)
	DEBUG_BREAK();
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
#endif
}


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