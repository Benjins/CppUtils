set -e

if [ -z "$CXX" ]; then
	echo "Could not find CXX variable, using g++ as default."
	CXX="g++"
fi

eval "$CXX -Wall -Wextra -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DREF_TEST_MAIN ref.cpp -o ref_test"
./ref_test
valgrind --leak-check=full --error-exitcode=12 ./ref_test

eval "$CXX -Wall -Wextra -std=c++11 -DBNS_DEBUG -DASSERT_TEST_MAIN assert.cpp -o assert_test"
yes b | head -n 20 | ./assert_test

eval "$CXX -Wall -Wextra -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DHASH_TEST_MAIN hash.cpp -o hash_test"
./hash_test

eval "$CXX -Wall -Wextra -g -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DVECTOR_TEST_MAIN vector.cpp -o vector_test"
valgrind --leak-check=full --error-exitcode=12 ./vector_test
