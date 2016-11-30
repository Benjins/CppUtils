set -e

if [ -z "$CXX" ]; then
	echo "Could not find CXX variable, using g++ as default."
	CXX="g++"
fi

VAGRIND_FLAGS="--quiet --leak-check=full --error-exitcode=12"
CXX_TEST_FLAGS="-Wall -Wextra -std=c++11 -DBNS_DEBUG -DEXIT_ON_ASSERT"

eval "$CXX $CXX_TEST_FLAGS -DREF_TEST_MAIN assert.cpp ref.cpp -o ref_test"
./ref_test
valgrind $VAGRIND_FLAGS ./ref_test

eval "$CXX -Wall -Wextra -std=c++11 -DBNS_DEBUG -DASSERT_TEST_MAIN assert.cpp -o assert_test"
yes b | head -n 20 | ./assert_test

eval "$CXX $CXX_TEST_FLAGS -DHASH_TEST_MAIN assert.cpp hash.cpp -o hash_test"
./hash_test

eval "$CXX $CXX_TEST_FLAGS -DDISC_UNION_TEST_MAIN assert.cpp vector.cpp strings.cpp disc_union.cpp -o disc_union_test"
valgrind $VAGRIND_FLAGS ./disc_union_test

eval "$CXX $CXX_TEST_FLAGS -DVECTOR_TEST_MAIN assert.cpp vector.cpp -o vector_test"
valgrind $VAGRIND_FLAGS ./vector_test

eval "$CXX $CXX_TEST_FLAGS -DFILESYS_TEST_MAIN assert.cpp filesys.cpp -o filesys_test"
./filesys_test

eval "$CXX $CXX_TEST_FLAGS -DIDBASE_TEST_MAIN assert.cpp idbase.cpp -o idbase_test"
valgrind $VAGRIND_FLAGS ./idbase_test

eval "$CXX $CXX_TEST_FLAGS -DSTRINGS_TEST_MAIN assert.cpp strings.cpp -o strings_test"
valgrind $VAGRIND_FLAGS ./strings_test

eval "$CXX $CXX_TEST_FLAGS -DSTRINGMAP_TEST_MAIN assert.cpp strings.cpp hash.cpp stringmap.cpp -o stringmap_test"
valgrind $VAGRIND_FLAGS ./stringmap_test

eval "$CXX $CXX_TEST_FLAGS -DMEMSTREAM_TEST_MAIN assert.cpp strings.cpp memstream.cpp stringmap.cpp hash.cpp -o memstream_test"
valgrind $VAGRIND_FLAGS ./memstream_test

eval "$CXX $CXX_TEST_FLAGS -DXML_TEST_MAIN strings.cpp assert.cpp hash.cpp xml.cpp -o xml_test"
valgrind $VAGRIND_FLAGS ./xml_test

eval "$CXX $CXX_TEST_FLAGS -DLEXER_TEST_MAIN lexer.cpp strings.cpp assert.cpp vector.cpp -o lex_test"
valgrind $VAGRIND_FLAGS ./lex_test

eval "$CXX $CXX_TEST_FLAGS -DBNVPARSER_TEST_MAIN lexer.cpp strings.cpp memstream.cpp assert.cpp hash.cpp vector.cpp bnvm.cpp bnvparser.cpp -o bnvparser_test"
valgrind $VAGRIND_FLAGS ./bnvparser_test

./bnvparser_test > bnvParserOut.txt
diff bnvParserOut.txt bnvParserExpectedOut.txt

eval "$CXX $CXX_TEST_FLAGS -DBNVM_TEST_MAIN lexer.cpp strings.cpp assert.cpp hash.cpp memstream.cpp stringmap.cpp bnvm.cpp vector.cpp bnvparser.cpp -o bnvm_test"
valgrind $VAGRIND_FLAGS ./bnvm_test

eval "$CXX $CXX_TEST_FLAGS -DCOMMANDLINE_TEST_MAIN commandline.cpp strings.cpp assert.cpp -o commandline_test.out"
valgrind $VAGRIND_FLAGS ./commandline_test.out

eval "$CXX $CXX_TEST_FLAGS -DUNICODE_TEST_MAIN unicode.cpp strings.cpp filesys.cpp assert.cpp -o unicode_test.out"
valgrind $VAGRIND_FLAGS ./unicode_test.out
diff unicode_test.txt unicode__out.txt

eval "$CXX $CXX_TEST_FLAGS -DSOCKET_TEST_MAIN socket.cpp assert.cpp -o socket_test.out"

cd Coroutines
chmod +x ./coroutine-test.sh
./coroutine-test.sh

cppcheck --force --inline-suppr --suppress=memsetClass --suppress=purgedConfiguration --suppress=noExplicitConstructor --suppress=cstyleCast --suppress=missingIncludeSystem --suppress=unusedFunction --error-exitcode=21 --template '{file},{line},{severity},{id},{message}' --quiet --verbose -DBNS_DEBUG -UREF_TEST_MAIN -UASSERT_TEST_MAIN -UHASH_TEST_MAIN -UVECTOR_TEST_MAIN -UFILESYS_TEST_MAIN -UIDBASE_TEST_MAIN -USTRINGS_TEST_MAIN -USTRINGMAP_TEST_MAIN -UMEMSTREAM_TEST_MAIN -UBNVPARSER_TEST_MAIN -UUBNVM_TEST_MAIN -UXML_TEST_MAIN -ULEXER_TEST_MAIN -USOCKET_TEST_MAIN -UCOMMANDLINE_TEST_MAIN --std=c++11 *.cpp
