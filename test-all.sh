set -e

if [ -z "$CXX" ]; then
	echo "Could not find CXX variable, using g++ as default."
	CXX="g++"
fi

VAGRIND_FLAGS="--quiet --leak-check=full --error-exitcode=12"
CXX_TEST_FLAGS="-Wall -Wextra -g -O0 -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -pthread"

eval "$CXX -Wall -Wextra -std=c++11 -DBNS_DEBUG -DASSERT_TEST_MAIN assert.cpp -o assert_test.out"
yes b | head -n 20 | ./assert_test.out


eval "$CXX $CXX_TEST_FLAGS all_tests.cpp -o all_tests.out"
./all_tests.out
valgrind $VAGRIND_FLAGS ./all_tests.out

cd Coroutines
chmod +x ./coroutine-test.sh
./coroutine-test.sh

cppcheck --force --inline-suppr --suppress=memsetClass --suppress=purgedConfiguration --suppress=noExplicitConstructor --suppress=cstyleCast --suppress=missingIncludeSystem --suppress=unusedFunction --error-exitcode=21 --template '{file},{line},{severity},{id},{message}' --quiet --verbose -DBNS_DEBUG -UASSERT_TEST_MAIN -UHASH_TEST_MAIN -UVECTOR_TEST_MAIN -UFILESYS_TEST_MAIN -UIDBASE_TEST_MAIN -USTRINGS_TEST_MAIN -USTRINGMAP_TEST_MAIN -UMEMSTREAM_TEST_MAIN -UBNVPARSER_TEST_MAIN -UUBNVM_TEST_MAIN -UXML_TEST_MAIN -ULEXER_TEST_MAIN -USOCKET_TEST_MAIN -UCOMMANDLINE_TEST_MAIN --std=c++11 *.cpp
