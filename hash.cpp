#include <stdio.h>
#include <string.h>

#include "macros.h"

#include "assert.cpp"


typedef unsigned int Hash;

Hash hash(void* mem, int size){
	unsigned char* data = (unsigned char*)mem;
	
	unsigned long hashVal = 0xcbf29ce484222325ULL;
	
	for(int i = 0; i < size; i++){
		hashVal ^= data[i];
		hashVal *= 0x100000001b3ULL;
	}
	
	return (Hash)hashVal;
}

Hash hash(char* str){
	return hash(str, strlen(str));
}


#if defined(HASH_TEST_MAIN)


int main(int argc, char** argv){
	
	int* counts = new int[48238123];
	
	for(unsigned int i = 0; i < 50000000; i++){
		Hash intHash = hash(&i, sizeof(i));
		
		int idx = intHash % 48238123;
		
		counts[idx]++;
	}
	
	for(unsigned int i = 0; i < 48238123; i++){
		ASSERT_MSG(counts[i] < 8, "Hash count for %d is %d, not within tolerance.", i, counts[i]);
	}
	
	delete[] counts;	

	while(false){
		printf("Enter a string.\n");

		char response[256] = {0};
		fgets(response, 255, stdin);

		for(int i = 0; i < 256; i++){
			if(response[i] == '\n' || response[i] == '\r'){
				response[i] = '\0';
			}
		}

		printf("String: '%s', hash: '%d'", response, hash(response));
	}

	return 0;
}


#endif

