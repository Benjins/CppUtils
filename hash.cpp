#include <stdio.h>
#include <string.h>

#include "macros.h"

#include "assert.cpp"


typedef unsigned int Hash;

Hash hash(void* mem, int size){
	unsigned char* data = (unsigned char*)mem;
	
	unsigned long hashVal = 1000009559;
	
	for(int i = 0; i < size; i++){
		hashVal ^= (data[i] * 48238123);
		hashVal = hashVal % 4000004387L;
	}
	
	return (Hash)hashVal;
}

Hash hash(char* str){
	return hash(str, strlen(str));
}


#if defined(HASH_TEST_MAIN)


int main(int argc, char** argv){
	
	int* counts = new int[50000];
	
	for(unsigned int i = 0; i < 50000000; i++){
		Hash intHash = hash(&i, sizeof(i));
		
		int idx = intHash % 49927;
		
		counts[idx]++;
	}
	
	for(unsigned int i = 0; i < 49927; i++){
		ASSERT_MSG(counts[i] > 500 && counts[i] < 2800, "Hash count for %d is %d, not within tolerance.", i, counts[i]);
	}
	
	delete[] counts;	
	return 0;
}


#endif

