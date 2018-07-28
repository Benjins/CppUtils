#include "macros.h"

#if defined(MACROS_TEST_MAIN)

#include "assert.cpp"
#include "strings.cpp"

struct testStruct {
	int x;
	int y;
};

int main(int argc, char** argv) {

	{
		int counter = 0;
		for (int i = 0; i < 10; i++) {
			BNS_DO_EXACTLY_ONCE() {
				counter++;
			}
		}

		ASSERT(counter == 1);
	}

	{
		// Ideally, these would be static asserts, but clang trips up on them...:\...
		ASSERT(BNS_OFFSET_OF(testStruct, x) == 0);

		//possible packing issue?
		ASSERT(BNS_OFFSET_OF(testStruct, y) == 4);
	}

	{
		int arr[3] = { 1, 2, 3 };
		static_assert(BNS_ARRAY_COUNT(arr) == 3, "check BNS_ARRAY_COUNT macro");
	}

	{
		int arr_23 = 0;
		BNS_GLUE_TOKS(arr, _23) = 5;
		BNS_GLUE_TOKS(arr_, 23) = 6;
		// No need to assert or anything.  This will fail to compile if there's an issue
	}

	{
		int arr1[3] = { 100, 1033, -23 };
		int arr2[3] = {};
		BNS_MEMCPY(arr2, arr1, sizeof(arr1));
		for (int i = 0; i < 3; i++) {
			ASSERT(arr1[i] == arr2[i]);
		}

		ASSERT(arr1[0] ==  100);
		ASSERT(arr1[1] == 1033);
		ASSERT(arr1[2] ==  -23);
	}

	static_assert(BNS_MIN(2, 3) == 2, "check BNS_MIN macro");
	static_assert(BNS_MAX(2, 3) == 3, "check BNS_MIN macro");

	static_assert(BNS_ABS(3)  == 3, "check BNS_ABS macro");
	static_assert(BNS_ABS(-3) == 3, "check BNS_ABS macro");
	static_assert(BNS_ABS(0)  == 0, "check BNS_ABS macro");
	
	static_assert(BNS_SQR(0)  == 0,  "check BNS_SQR macro");
	static_assert(BNS_SQR(1)  == 1,  "check BNS_SQR macro");
	static_assert(BNS_SQR(-1) == 1,  "check BNS_SQR macro");
	static_assert(BNS_SQR(2)  == 4,  "check BNS_SQR macro");
	static_assert(BNS_SQR(-2) == 4,  "check BNS_SQR macro");
	static_assert(BNS_SQR(-6) == 36, "check BNS_SQR macro");
	static_assert(BNS_SQR(6)  == 36, "check BNS_SQR macro");

	{
		ASSERT(StrEqual(BNS_STRINGIFY(the), "the"));
		static_assert(sizeof(BNS_STRINGIFY(the)) == 4, "check BNS_STRINGIFY macro");
	}

	return 0;
}


#endif

