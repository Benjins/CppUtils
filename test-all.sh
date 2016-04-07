set -e

if [ -z "$CXX" ]; then
	echo "Could not find CXX variable, using g++ as default."
	CXX="g++"
fi

eval "$CXX -Wall -Wextra -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DREF_TEST_MAIN assert.cpp ref.cpp -o ref_test"
./ref_test
valgrind --quiet --leak-check=full --error-exitcode=12 ./ref_test

eval "$CXX -Wall -Wextra -std=c++11 -DBNS_DEBUG -DASSERT_TEST_MAIN assert.cpp -o assert_test"
yes b | head -n 20 | ./assert_test

eval "$CXX -Wall -Wextra -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DHASH_TEST_MAIN assert.cpp hash.cpp -o hash_test"
./hash_test

eval "$CXX -Wall -Wextra -g -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DVECTOR_TEST_MAIN assert.cpp vector.cpp -o vector_test"
valgrind --quiet --leak-check=full --error-exitcode=12 ./vector_test

eval "$CXX -Wall -Wextra -g -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DFILESYS_TEST_MAIN assert.cpp filesys.cpp -o filesys_test"
./filesys_test

eval "$CXX -Wall -Wextra -g -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DIDBASE_TEST_MAIN assert.cpp idbase.cpp -o idbase_test"
valgrind --quiet --leak-check=full --error-exitcode=12 ./idbase_test

eval "$CXX -Wall -Wextra -g -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DSTRINGS_TEST_MAIN assert.cpp strings.cpp -o strings_test"
valgrind --quiet --leak-check=full --error-exitcode=12 ./strings_test
