@echo off

g++ -Wall -Wextra -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DREF_TEST_MAIN ref.cpp -o ref_test
if errorlevel 1 goto somethingbad

ref_test.exe
if errorlevel 1 goto somethingbad

g++ -Wall -Wextra -std=c++11 -DBNS_DEBUG -DASSERT_TEST_MAIN assert.cpp -o assert_test
if errorlevel 1 goto somethingbad

assert_test.exe < b-all.txt
if errorlevel 1 goto somethingbad

g++ -Wall -Wextra -g -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DHASH_TEST_MAIN hash.cpp -o hash_test
if errorlevel 1 goto somethingbad

hash_test.exe
if errorlevel 1 goto somethingbad

echo Success!
goto EOF
:somethingbad
echo Something Bad Happened.
:EOF