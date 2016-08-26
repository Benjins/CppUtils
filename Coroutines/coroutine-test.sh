set -e

if [ -z "$CXX" ] ; then
	echo "Could not find CXX variable, using g++ as default."
	CXX=g++
fi

eval "$CXX -Wall -Wextra -g -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DCOROUTINE_MAIN ../vector.cpp ../assert.cpp ../strings.cpp ../lexer.cpp coroutines.cpp -o coroutines.out"

./coroutines.out

eval "$CXX -Wall -Wextra -g -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DCOROUTINE_TEST_MAIN ../assert.cpp coroutines_example_use.cpp -o coroutines_example_use.out"

./coroutines_example_use.out
