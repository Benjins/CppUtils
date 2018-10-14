
touch ci_build_results.txt


## Build both clang + gcc, but gcc makes more sense to test since this is meant to run on Linux
clang++ -Og -g -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -pthread all_tests.cpp -o all_tests.out 2> ci_build_results.txt

g++ -Og -g -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -pthread all_tests.cpp -o all_tests.out 2>> ci_build_results.txt


./all_tests.out --output-results

