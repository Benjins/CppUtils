@echo off

g++ -Wall -Wextra -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DREF_TEST_MAIN assert.cpp ref.cpp -o ref_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

ref_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

g++ -Wall -Wextra -std=c++11 -DBNS_DEBUG -DASSERT_TEST_MAIN assert.cpp -o assert_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

assert_test.exe < b-all.txt
if %ERRORLEVEL% NEQ 0 goto somethingbad

g++ -Wall -Wextra -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DHASH_TEST_MAIN assert.cpp hash.cpp -o hash_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

hash_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DVECTOR_TEST_MAIN assert.cpp vector.cpp /Fevector_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

vector_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DFILESYS_TEST_MAIN assert.cpp filesys.cpp /Fefilesys_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

filesys_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DIDBASE_TEST_MAIN assert.cpp idbase.cpp /Feidbase_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

idbase_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DSTRINGS_TEST_MAIN assert.cpp strings.cpp /Festrings_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

strings_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DSTRINGMAP_TEST_MAIN assert.cpp hash.cpp strings.cpp stringmap.cpp /Festringmap_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

stringmap_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT__ON_ASSERT /DCOMBINEDTESTS_TEST_MAIN stringmap.cpp strings.cpp hash.cpp assert.cpp combinedtest.cpp /Fecombined_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

combined_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

echo Success!
goto :EOF
:somethingbad
echo Something Bad Happened.