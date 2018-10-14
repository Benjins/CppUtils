#ifndef TESTING_H_
#define TESTING_H_

#pragma once

typedef int (TestRunFunc)();

void RegisterTest(const char* name, const char* file, int line, TestRunFunc* testFunc);

struct _TestRegisterObject {
	_TestRegisterObject(const char* name, const char* file, int line, TestRunFunc* testFunc) {
		RegisterTest(name, file, line, testFunc);
	}
};

#define CREATE_TEST_CASE_INT(name, cnt) \
					int _BNS_TEST_FUNC_ ## cnt (); \
					_TestRegisterObject _bns_test_register ## cnt(name, __FILE__, __LINE__, _BNS_TEST_FUNC_ ## cnt); \
					int _BNS_TEST_FUNC_ ## cnt ()
#define CREATE_TEST_CASE_INT_(name, cnt) CREATE_TEST_CASE_INT(name, cnt)

#define CREATE_TEST_CASE(name) CREATE_TEST_CASE_INT_(name, __COUNTER__)

/*

Usage: 

CREATE_TEST_CASE("Check numbers") {
	int x = 3 + 5;
	ASSERT(x == 8);
}



*/

int RunAllTests();

#endif
