#include <stdio.h>
#include <string.h>
#include <new>

#include "macros.h"
#include "assert.h"

template<typename T>
struct Vector{
	T* data;
	int count;
	int capacity;
	
	Vector(){
		data = nullptr;
		count = 0;
		capacity = 0;
	}
	
	Vector(const Vector& other){
		data = (T*)malloc(other.capacity*sizeof(T));
		count = other.count;
		capacity = other.capacity;
		for(int i = 0; i < other.count; i++){
			new(&data[i]) T(other.data[i]);
		}
	}
	
	void EnsureCapacity(int newCapacity){
		if(newCapacity > capacity){
			T* newData = (T*)malloc(newCapacity*sizeof(T));
			memcpy(newData, data, count*sizeof(T));
			free(data);
			
			data = newData;
			capacity = newCapacity;
		}
	}
	
	void Clear(){
		for(int i = 0; i < count; i++){
			data[i].~T();
		}
		
		count = 0;
	}
	
	void PopBack(){
		ASSERT(count > 0);
		count--;
		data[count].~T();
	}
	
	void Remove(int index){
		ASSERT(index >= 0 && index < count);
		data[index].~T();
		memmove(&data[index], &data[index+1], sizeof(T)*(count - index - 1));
		count--;
	}
	
	void PushBack(const T& elem){
		if(count >= capacity){
			EnsureCapacity(capacity == 0 ? 2 : capacity * 2);
		}
		
		new(&data[count]) T(elem);
		count++;
	}
	
	void Insert(int index, const T& elem){
		if(count >= capacity){
			EnsureCapacity(capacity == 0 ? 2 : capacity * 2);
		}
		
		unsigned char* insertLoc = (unsigned char*)&data[index];
		for(int i = sizeof(T) * count - index - 1; i >= 0; i--){
			insertLoc[i + sizeof(T)] = insertLoc[i];
		}
		
		new(&data[index]) T(elem);
		count++;
	}
	
	void Destroy(){
		Clear();
		
		if(data != nullptr){
			free(data);
			data = nullptr;
		}
		else{
			ASSERT(capacity == 0);
		}
		
		capacity = 0;
	}
	
	Vector(int initialCapacity){
		count = 0;
		capacity = 0;
		data = nullptr;
		EnsureCapacity(initialCapacity);
	}
	
	T& Get(int idx){
		ASSERT(idx >= 0 && idx < count);
		return data[idx];
	}
	
	~Vector(){
		Destroy();
	}
};

#if defined(VECTOR_TEST_MAIN)

//Used to track potential resource leaks in the vector test
int fakeAllocCount = 0;

struct Resource{
	int pad;
	
	Resource(){
		fakeAllocCount++;
	}
	
	Resource(const Resource& orig){
		fakeAllocCount++;
		BNS_UNUSED(orig);
	}
	
	~Resource(){
		fakeAllocCount--;
	}
};

int main(int argc, char** argv){
	BNS_UNUSED(argc);
	BNS_UNUSED(argv);

	Vector<int> iVec(20);
	
	Vector<int> iVec2;
	iVec2.PushBack(34);
	iVec2.PushBack(33);
	iVec2.PushBack(32);
	iVec2.PushBack(31);
	
	ASSERT(iVec2.count == 4);
	ASSERT(iVec2.Get(0) == 34);
	ASSERT(iVec2.Get(1) == 33);
	ASSERT(iVec2.Get(2) == 32);
	ASSERT(iVec2.Get(3) == 31);
	
	iVec2.Insert(2, 37);
	ASSERT(iVec2.count == 5);
	ASSERT(iVec2.Get(0) == 34);
	ASSERT(iVec2.Get(1) == 33);
	ASSERT(iVec2.Get(2) == 37);
	ASSERT(iVec2.Get(3) == 32);
	ASSERT(iVec2.Get(4) == 31);
	
	Vector<int> iVec3 = iVec2;
	ASSERT(iVec3.count == 5);
	ASSERT(iVec3.Get(0) == 34);
	ASSERT(iVec3.Get(1) == 33);
	ASSERT(iVec3.Get(2) == 37);
	ASSERT(iVec3.Get(3) == 32);
	ASSERT(iVec3.Get(4) == 31);
	
	iVec3.EnsureCapacity(25);
	iVec3.PushBack(22);
	
	ASSERT(iVec2.count == 5);
	ASSERT(iVec3.count == 6);
	ASSERT(iVec3.Get(5) == 22);
	
	iVec3.Clear();
	ASSERT(iVec3.count == 0);
	
	Vector<int>(39).Destroy();
	
	Vector<int> iVec4;
	iVec4.PushBack(23);
	iVec4.PushBack(24);
	iVec4.PushBack(25);
	ASSERT(iVec4.count == 3);
	
	for(int i = 0; i < 3; i++){
		iVec4.PopBack();
		ASSERT(iVec4.count == 2 - i);
	}
	
	void* oldData = iVec4.data;
	iVec4.PushBack(23);
	iVec4.PushBack(24);
	iVec4.PushBack(25);
	ASSERT(iVec4.count == 3);
	//No re-allocation should happen
	ASSERT(iVec4.data == oldData);
	
	iVec4.Remove(1);
	ASSERT(iVec4.count == 2);
	ASSERT(iVec4.Get(0) == 23);
	ASSERT(iVec4.Get(1) == 25);
	
	const int testCount = 5000;
	Vector<int> iVec5;
	for(int i = 0; i < testCount; i++){
		iVec5.PushBack(i);
	}
	
	ASSERT(iVec5.count == testCount);
	
	Vector<int> iVec6 = iVec5;
	
	for(int i = 0; i < testCount; i++){
		iVec5.PopBack();
	}

	ASSERT(iVec5.count == 0);
	
	for(int i = 0; i < testCount; i++){
		iVec6.Remove(0);
	}
	
	ASSERT(iVec6.count == 0);
	
	ASSERT(fakeAllocCount == 0);
	
	{
		Vector<Resource> rVec(5);
	}
	
	ASSERT(fakeAllocCount == 0);
	
	{
		Vector<Resource> rVec;
		
		for(int i = 0; i < 20; i++){
			Resource r;
			rVec.PushBack(r);
		}
	}
	
	ASSERT(fakeAllocCount == 0);
	
	{
		Vector<Resource> rVec;
		
		for(int i = 0; i < 20; i++){
			Resource r;
			rVec.PushBack(r);
		}
		
		rVec.Clear();
		
		ASSERT(fakeAllocCount == 0);
	}
	
	{
		Vector<Resource> rVec;
		
		for(int i = 0; i < 20; i++){
			Resource r;
			rVec.PushBack(r);
		}
		
		rVec.Clear();
		
		Vector<Resource> rVecCpy = rVec;
	}
	
	ASSERT(fakeAllocCount == 0);
	
	return 0;
}

#endif
