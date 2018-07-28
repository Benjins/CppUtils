@echo off

g++ -Wall -Wextra -std=c++11 -DBNS_DEBUG -DASSERT_TEST_MAIN assert.cpp -o assert_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

assert_test.exe < b-all.txt
if %ERRORLEVEL% NEQ 0 goto somethingbad

g++ -Wall -Wextra -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT -DHASH_TEST_MAIN hash.cpp -o hash_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

hash_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DVECTOR_TEST_MAIN vector.cpp /Fevector_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

vector_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DMACROS_TEST_MAIN macros_test.cpp /Femacros_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

macros_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DTHREADS_TEST_MAIN threads.cpp /Fethreads_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

threads_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DDISC_UNION_TEST_MAIN disc_union.cpp /Fedisc_union_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

disc_union_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DFILESYS_TEST_MAIN filesys.cpp /Fefilesys_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

filesys_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DIDBASE_TEST_MAIN idbase.cpp /Feidbase_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

idbase_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DSTRINGS_TEST_MAIN strings.cpp /Festrings_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

strings_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DBNSEXPR_TEST_MAIN sexpr.cpp /Fesexpr_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

sexpr_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DSTRINGMAP_TEST_MAIN stringmap.cpp /Festringmap_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

stringmap_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DCOMBINEDTESTS_TEST_MAIN combinedtest.cpp /Fecombined_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

combined_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DMEMSTREAM_TEST_MAIN memstream.cpp /Fememstream_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

memstream_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DXML_TEST_MAIN xml.cpp /Fexml_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

xml_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DLEXER_TEST_MAIN lexer.cpp /Felex_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

lex_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DBNVPARSER_TEST_MAIN bnvparser.cpp /Febnvparser_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

bnvparser_test.exe > bnvParserOut.txt
if %ERRORLEVEL% NEQ 0 goto somethingbad

diff --strip-trailing-cr bnvParserOut.txt bnvParserExpectedOut.txt
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DBNVM_TEST_MAIN bnvm.cpp /Febnvm_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

bnvm_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DCOMMANDLINE_TEST_MAIN commandline.cpp /Fecommandline_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

commandline_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DUNICODE_TEST_MAIN unicode.cpp /Feunicode_test
if %ERRORLEVEL% NEQ 0 goto somethingbad

unicode_test.exe
if %ERRORLEVEL% NEQ 0 goto somethingbad

cl /Od /Zi /DBNS_DEBUG /DEXIT_ON_ASSERT /DSOCKET_TEST_MAIN socket.cpp /Fesocket_test

pushd .

cd Coroutines
call coroutine-test.bat
if %ERRORLEVEL% NEQ 0 goto somethingbad

popd

echo Success!
goto :EOF
:somethingbad
echo Something Bad Happened.