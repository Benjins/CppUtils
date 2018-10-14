#include "testing.h"


#define BNVM_TEST_MAIN
#define BNVPARSER_TEST_MAIN
#define COMBINEDTESTS_TEST_MAIN
#define COMMANDLINE_TEST_MAIN
#define DISC_UNION_TEST_MAIN
#define FILESYS_TEST_MAIN
#define FIXEDPOOL_TEST_MAIN
#define HASH_TEST_MAIN
#define IDBASE_TEST_MAIN
#define LEXER_TEST_MAIN
#define MACROS_TEST_MAIN
#define MEMSTREAM_TEST_MAIN
#define BNSEXPR_TEST_MAIN
#define SOCKET_TEST_MAIN
#define STRINGMAP_TEST_MAIN
#define STRINGS_TEST_MAIN
#define THREADS_TEST_MAIN
#define UNICODE_TEST_MAIN
#define VECTOR_TEST_MAIN
#define XML_TEST_MAIN

// TODO: also include assert test?
#include "assert.cpp"
#include "bitset.cpp"
#include "bnvm.cpp"
#include "bnvparser.cpp"
#include "combinedtest.cpp"
#include "commandline.cpp"
#include "disc_union.cpp"
#include "filesys.cpp"
#include "fixedpool.cpp"
#include "hash.cpp"
#include "idbase.cpp"
#include "lexer.cpp"
#include "macros_test.cpp"
#include "memstream.cpp"
#include "sexpr.cpp"
// TODO:
//#include "socket.cpp"
#include "stringmap.cpp"
#include "strings.cpp"
#include "threads.cpp"
#include "unicode.cpp"
#include "vector.cpp"
#include "xml.cpp"

#define CREATE_TEST_MAIN
#include "testing.cpp"