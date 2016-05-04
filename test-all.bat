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

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DCOMBINEDTESTS_TEST_MAIN stringmap.cpp strings.cpp hash.cpp assert.cpp combinedtest.cpp /Fecombined_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

combined_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DMEMSTREAM_TEST_MAIN strings.cpp assert.cpp memstream.cpp /Fememstream_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

memstream_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DXML_TEST_MAIN strings.cpp assert.cpp hash.cpp xml.cpp /Fexml_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

xml_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DLEXER_TEST_MAIN lexer.cpp strings.cpp assert.cpp vector.cpp /Felex_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

lex_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT__ON_ASSERT /DBNVPARSER_TEST_MAIN lexer.cpp strings.cpp assert.cpp hash.cpp vector.cpp bnvm.cpp bnvparser.cpp /Febnvparser_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

bnvparser_test.exe 
::> bnvParserOut.txt
if %ERRORLEVEL% NEQ 0 goto somethingbad

diff bnvParserOut.txt bnvParserExpectedOut.txt
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DBNVM_TEST_MAIN lexer.cpp strings.cpp assert.cpp hash.cpp stringmap.cpp bnvm.cpp vector.cpp bnvparser.cpp /Febnvm_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

bnvm_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

echo Success!
goto :EOF
:somethingbad
echo Something Bad Happened.