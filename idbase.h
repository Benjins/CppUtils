#ifndef ID_BASE_H
#define ID_BASE_H

#include <type_traits>
#include <new>
#include <string.h>

#include "assert.h"

typedef unsigned int uint32;

struct IDBase{
	uint32 id;
};

template<typename T>
struct IDTracker{
	T* vals;
	int maxCount;
	int currentCount;
	uint32 currentMaxId;
	
	static_assert(std::is_base_of<IDBase, T>::value, "IDTracker must be instantiated with a child of IDBase.");
	
	IDTracker(uint32 _maxCount = 0){
		maxCount = _maxCount;
		currentCount = 0;
		currentMaxId = 0;
		
		if(maxCount > 0){
			vals = (T*)malloc(sizeof(T) * maxCount);
		}
		else{
			vals = nullptr;
		}
	}
	
	IDTracker(const IDTracker& other){
		maxCount = other.maxCount;
		currentCount = other.currentCount;
		currentMaxId = other.currentMaxId;
		vals = (T*)malloc(maxCount*sizeof(T));

		for(int i = 0; i < currentCount; i++){
			new (&vals[i]) T(other.vals[i]);
		}
	}
	
	void SetSize(uint32 newSize){
		if(currentCount > 0){
			// TODO: Should we warn here?
		}
		
		maxCount = newSize;
		
		if(vals != nullptr){
			T* newVals = (T*)malloc(maxCount*sizeof(T));
			
			for(int i = 0; i < currentCount; i++){
				newVals[i] = vals[i];
			}
			
			for(int i = 0; i < currentCount; i++){
				vals[i].~T();
			}
			
			free(vals);
			
			vals = newVals;
		}
		else{
			currentCount = 0;
			currentMaxId = 0;
			
			if(maxCount > 0){
				vals = (T*)malloc(maxCount*sizeof(T));
			}
		}
	}
	
	T* CreateAndAdd(){
		ASSERT(currentCount < maxCount);
		
		T* ptr = new(&vals[currentCount]) T();
		vals[currentCount].id = currentMaxId;
		currentCount++;
		currentMaxId++;
		
		return ptr;
	}
	
	T* GetById(uint32 id){
		for(int i = 0; i < currentCount; i++){
			if(vals[i].id == id){
				return &vals[i];
			}
		}
		
		return nullptr;
	}
	
	void RemoveById(uint32 id){
		for(int i = 0; i < currentCount; i++){
			if(vals[i].id == id){
				vals[i].~T();
				memmove(&vals[i], &vals[i+1], sizeof(T) * (currentCount - i - 1));
				currentCount--;
				break;
			}
		}
	}
	
	~IDTracker(){
		if(vals != nullptr){
			
			for(int i = 0; i < currentCount; i++){
				vals[i].~T();
			}
			
			free(vals);
		}
	}
};

#endif
