#include "testing.h"

#include "vector.h"

struct TestInfo {
	TestRunFunc* func;
	const char* name;
	const char* file;
	int line;
};

#define TEST_MAX_COUNT (64 * 1024)
static int _test_info_to_run_count = 0;
TestInfo _test_info_to_run[TEST_MAX_COUNT];

void RegisterTest(const char* name, const char* file, int line, TestRunFunc* testFunc) {
	TestInfo info;
	info.func = testFunc;
	info.name = name;
	info.file = file;
	info.line = line;

	_test_info_to_run[_test_info_to_run_count] = info;
	_test_info_to_run_count++;
}

int RunAllTests(bool outputResults) {
	FILE* testResults = NULL;
	if (outputResults) {
		testResults = fopen("test_results.txt", "wb");
		
		for (int i = 0; i < _test_info_to_run_count; i++) {
			auto* ptr = &_test_info_to_run[i];
			fprintf(testResults, "E$%s$%s$%d\n", ptr->name, ptr->file, ptr->line);
		}
		
		fflush(testResults);
	}
	
	for (int i = 0; i < _test_info_to_run_count; i++) {
		auto* ptr = &_test_info_to_run[i];

		// TODO: Try catch to get failures from any assertions...
		int rc = (*ptr->func)();
		
		if (outputResults) {
			if (rc == 0) {
				fprintf(testResults, "P$%s$%s$%d\n", ptr->name, ptr->file, ptr->line);
			} else {
				fprintf(testResults, "F$%s$%s$%d\n", ptr->name, ptr->file, ptr->line);
			}
			
			fflush(testResults);
		}
	}
	
	if (outputResults) {
		fclose(testResults);
	}
	
	return 0;
}

#if defined(CREATE_TEST_MAIN)

int main(int argc, char** argv) {
	
	bool outputResults = false;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--output-results") == 0) {
			outputResults = true;
			break;
		}
	}
	
	return RunAllTests(outputResults);
}

#endif