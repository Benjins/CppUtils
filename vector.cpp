#include <stdio.h>
#include <string.h>
#include <new>

#include "macros.h"
#include "assert.cpp"

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

int main(int argc, char** argv){
	
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
	
	return 0;
}

#endif
