#include <stdio.h>
#include <string.h>

#include "hash.h"

#include "macros.h"
#include "assert.h"


typedef unsigned int Hash;

#if defined(_MSC_VER)
typedef unsigned long long uint64;
#else
typedef unsigned long uint64;
#endif

Hash ComputeHash(const void* mem, int size){
	const unsigned char* data = (const unsigned char*)mem;
	
	uint64 hashVal = 0xcbf29ce484222325ULL;
	
	for(int i = 0; i < size; i++){
		hashVal ^= data[i];
		hashVal *= 0x100000001b3ULL;
	}
	
	return (Hash)hashVal;
}

Hash ComputeHash(const char* str){
	return ComputeHash(str, (int)strlen(str));
}


#if defined(HASH_TEST_MAIN)


int main(int argc, char** argv){
	BNS_UNUSED(argc);
	BNS_UNUSED(argv);
	
	int* counts = new int[48238123];
	
	for(unsigned int i = 0; i < 5000000; i++){
		Hash intHash = ComputeHash(&i, sizeof(i));
		
		int idx = intHash % 48238123;
		
		counts[idx]++;
	}
	
	for(unsigned int i = 0; i < 48238123; i++){
		ASSERT_MSG(counts[i] < 3, "Hash count for %d is %d, not within tolerance.", i, counts[i]);
	}
	
	delete[] counts;	

	return 0;
}


#endif

