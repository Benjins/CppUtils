set -e

if [ -z "$CXX" ]; then
	echo "Could not find CXX variable, using g++ as default."
	CXX="g++"
fi

VAGRIND_FLAGS="--quiet --leak-check=full --error-exitcode=12"
CXX_TEST_FLAGS="-Wall -Wextra -g -O0 -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT"

eval "$CXX -Wall -Wextra -std=c++11 -DBNS_DEBUG -DASSERT_TEST_MAIN assert.cpp -o assert_test.out"
yes b | head -n 20 | ./assert_test.out

eval "$CXX $CXX_TEST_FLAGS -DHASH_TEST_MAIN hash.cpp -o hash_test.out"
./hash_test.out

eval "$CXX $CXX_TEST_FLAGS -DMACROS_TEST_MAIN macros_test.cpp -o macros_test.out"
./macros_test.out

eval "$CXX $CXX_TEST_FLAGS -DDISC_UNION_TEST_MAIN disc_union.cpp -o disc_union_test.out"
valgrind $VAGRIND_FLAGS ./disc_union_test.out

eval "$CXX $CXX_TEST_FLAGS -DVECTOR_TEST_MAIN vector.cpp -o vector_test.out"
valgrind $VAGRIND_FLAGS ./vector_test.out

eval "$CXX $CXX_TEST_FLAGS -DFILESYS_TEST_MAIN filesys.cpp -o filesys_test.out"
valgrind $VAGRIND_FLAGS ./filesys_test.out

eval "$CXX $CXX_TEST_FLAGS -DTHREADS_TEST_MAIN threads.cpp -lpthread -o threads_test.out"
./threads_test.out

eval "$CXX $CXX_TEST_FLAGS -DIDBASE_TEST_MAIN idbase.cpp -o idbase_test.out"
valgrind $VAGRIND_FLAGS ./idbase_test.out

eval "$CXX $CXX_TEST_FLAGS -DSTRINGS_TEST_MAIN strings.cpp -o strings_test.out"
valgrind $VAGRIND_FLAGS ./strings_test.out

eval "$CXX $CXX_TEST_FLAGS -DBNSEXPR_TEST_MAIN sexpr.cpp -o sexpr_test.out"
valgrind $VAGRIND_FLAGS ./sexpr_test.out

eval "$CXX $CXX_TEST_FLAGS -DSTRINGMAP_TEST_MAIN stringmap.cpp -o stringmap_test.out"
valgrind $VAGRIND_FLAGS ./stringmap_test.out

eval "$CXX $CXX_TEST_FLAGS -DMEMSTREAM_TEST_MAINmemstream.cpp -o memstream_test.out"
valgrind $VAGRIND_FLAGS ./memstream_test.out

eval "$CXX $CXX_TEST_FLAGS -DXML_TEST_MAIN xml.cpp -o xml_test.out"
valgrind $VAGRIND_FLAGS ./xml_test.out

eval "$CXX $CXX_TEST_FLAGS -DLEXER_TEST_MAIN lexer.cpp -o lex_test.out"
valgrind $VAGRIND_FLAGS ./lex_test.out

eval "$CXX $CXX_TEST_FLAGS -DBNVPARSER_TEST_MAIN bnvparser.cpp -o bnvparser_test.out"
valgrind $VAGRIND_FLAGS ./bnvparser_test.out

./bnvparser_test.out > bnvParserOut.txt
diff bnvParserOut.txt bnvParserExpectedOut.txt

eval "$CXX $CXX_TEST_FLAGS -DBNVM_TEST_MAIN bnvm.cpp -o bnvm_test.out"
valgrind $VAGRIND_FLAGS ./bnvm_test.out

eval "$CXX $CXX_TEST_FLAGS -DCOMMANDLINE_TEST_MAIN commandline.cpp -o commandline_test.out"
valgrind $VAGRIND_FLAGS ./commandline_test.out

eval "$CXX $CXX_TEST_FLAGS -DUNICODE_TEST_MAIN unicode.cpp -o unicode_test.out"
valgrind $VAGRIND_FLAGS ./unicode_test.out
diff unicode_test.txt unicode__out.txt

eval "$CXX $CXX_TEST_FLAGS -DSOCKET_TEST_MAIN socket.cpp -o socket_test.out"

cd Coroutines
chmod +x ./coroutine-test.sh
./coroutine-test.sh

cppcheck --force --inline-suppr --suppress=memsetClass --suppress=purgedConfiguration --suppress=noExplicitConstructor --suppress=cstyleCast --suppress=missingIncludeSystem --suppress=unusedFunction --error-exitcode=21 --template '{file},{line},{severity},{id},{message}' --quiet --verbose -DBNS_DEBUG -UASSERT_TEST_MAIN -UHASH_TEST_MAIN -UVECTOR_TEST_MAIN -UFILESYS_TEST_MAIN -UIDBASE_TEST_MAIN -USTRINGS_TEST_MAIN -USTRINGMAP_TEST_MAIN -UMEMSTREAM_TEST_MAIN -UBNVPARSER_TEST_MAIN -UUBNVM_TEST_MAIN -UXML_TEST_MAIN -ULEXER_TEST_MAIN -USOCKET_TEST_MAIN -UCOMMANDLINE_TEST_MAIN --std=c++11 *.cpp
