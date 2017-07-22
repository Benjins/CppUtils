#include "fixedpool.h"
#include "assert.h"

FixedPool::FixedPool(){
	capacity = 0;
	allocSize = 0;
	mem = nullptr;
}

void FixedPool::Setup(int _allocSize, int count){
	capacity = count;
	allocSize = _allocSize;
	mem = malloc(allocSize * count);
	allocTracker.EnsureCapacity(count);
	allocTracker.Clear();
}

void* FixedPool::Allocate(){
	for (int i = 0; i < allocTracker.bytesAlloc/4; i++){
		if (((unsigned int)allocTracker.values[i]) != 0xFFFFFFFF){
			for (int j = 0; j < 32; j++){
				if ((allocTracker.values[i] & (1 << j)) == 0){
					int index = i * 32 + j;
					allocTracker.SetBit(i, true);
					void* ptr = ((char*)mem) + (index*allocSize);
					return ptr;
				}
			}

			ASSERT_WARN("%s", "Issue with alloc tracker, very strange.");
		}
	}

	ASSERT_WARN("%s", "Could not allocate out of fixed pool.");
	return nullptr;
}

void FixedPool::Deallocate(void* ptr){
	int index = (int)(((char*)ptr - (char*)mem) / allocSize);

	//Pointers should never be unaligned
	ASSERT((char*)mem + (index * allocSize) == ptr);
	ASSERT(index >= 0 && index < capacity);

	allocTracker.SetBit(index, false);
}

#if defined(FIXEDPOOL_TEST_MAIN)

#include "macros.h"

int main(){
	FixedPool pool;
	pool.Setup((int)sizeof(int), 70);

	for (int t = 0; t < 20; t++){
		void* allocs[68] = {};
		for (int i = 0; i < BNS_ARRAY_COUNT(allocs); i++){
			allocs[i] = pool.Allocate();
		}

		for (int i = 0; i < BNS_ARRAY_COUNT(allocs); i++){
			pool.Deallocate(allocs[i]);
		}
	}
	
	for (int i = 0; i < 70; i++) {
		ASSERT(!pool.allocTracker.GetBit(i));
	}

	return 0;
}



#endif

