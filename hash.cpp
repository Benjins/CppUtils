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
	return ComputeHash(str, str ? (int)strlen(str) : 0);
}


#if defined(HASH_TEST_MAIN)

CREATE_TEST_CASE("Hash collisions check") {	
	const int outputCount = 48238123;

	int* counts = new int[outputCount];
	for (int i = 0; i < outputCount; i++) {
		counts[i] = 0;
	}
	
	for(unsigned int i = 0; i < 5000000; i++){
		Hash intHash = ComputeHash(&i, sizeof(i));
		
		int idx = intHash % outputCount;
		
		counts[idx]++;
	}
	
	for(unsigned int i = 0; i < outputCount; i++){
		ASSERT_MSG(counts[i] < 3, "Hash count for %d is %d, not within tolerance.", i, counts[i]);
	}
	
	delete[] counts;	

	return 0;
}


#endif

