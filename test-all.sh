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

eval "$CXX -Wall -Wextra -g -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DSTRINGMAP_TEST_MAIN assert.cpp strings.cpp hash.cpp stringmap.cpp -o stringmap_test"
valgrind --quiet --leak-check=full --error-exitcode=12 ./stringmap_test

cppcheck --force --inline-suppr --suppress=cstyleCast --suppress=missingIncludeSystem --suppress=unusedFunction --error-exitcode=12 --template '{file},{line},{severity},{id},{message}' --quiet --verbose -DBNS_DEBUG -UREF_TEST_MAIN -UASSERT_TEST_MAIN -UHASH_TEST_MAIN -UVECTOR_TEST_MAIN -UFILESYS_TEST_MAIN -UIDBASE_TEST_MAIN -USTRINGS_TEST_MAIN -USTRINGMAP_TEST_MAIN --enable=all --std=c++11 *.cpp
