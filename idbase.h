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
struct IDHandle {
	int id;

	explicit IDHandle(uint32 _id = 0xFFFFFFFF) {
		id = _id;
	}

	IDHandle(const IDHandle<T>& orig) {
		id = orig.id;
	}

	IDHandle& operator=(const IDHandle<T> orig) {
		id = orig.id;
		return *this;
	}

	bool operator==(const IDHandle<T> orig) const {
		return orig.id == id;
	}
};

static_assert(sizeof(IDHandle<int>) == sizeof(int), "IDHandle must be the proper size.");

#define GET_HANDLE(obj) IDHandle<decltype(obj)>((obj).id)
#define GET_PTR_HANDLE(obj) IDHandle<std::remove_reference<decltype(*(obj))>::type>((obj)->id)

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
	
	// For now, we disallow copying
	IDTracker(const IDTracker& other) = delete;
	void operator=(const IDTracker& other) = delete;
	
	void Reset() {
		for (int i = 0; i < currentCount; i++) {
			vals[i].~T();
		}
		currentCount = 0;
		currentMaxId = 0;
	}

	void SetSize(uint32 newSize){
		maxCount = newSize;
		
		if(vals != nullptr){
			T* newVals = (T*)malloc(maxCount*sizeof(T));
			memset(newVals, 0xFE, maxCount*sizeof(T));
			
			for(int i = 0; i < currentCount; i++){
				new(&newVals[i]) T(vals[i]);
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
		if (currentCount >= maxCount) {
			SetSize(maxCount > 0 ? maxCount + 2 : 2);
		}
		
		T* ptr = new(&vals[currentCount]) T();
		vals[currentCount].id = currentMaxId;
		currentCount++;
		currentMaxId++;
		
		return ptr;
	}

	T* AddWithId(uint32 id) {
		ASSERT(GetByIdNum(id) == nullptr);

		if (currentCount >= maxCount) {
			SetSize(maxCount > 0 ? maxCount * 2 : 2);
		}

		T* ptr;
		if (id >= currentMaxId) {
			ptr = new(&vals[currentCount]) T();
			vals[currentCount].id = id;
			currentCount++;
			currentMaxId = id + 1;
		}
		else {
			for (int i = 0; i < currentCount; i++) {
				if (vals[i].id > id) {
					memmove(&vals[i + 1], &vals[i], (currentCount - i) * sizeof(T));
					ptr = new(&vals[i]) T();
					vals[i].id = id;
					currentCount++;

					break;
				}
			}
		}

		return ptr;
	}

	T* GetByIdNum(uint32 id) {
		for(int i = 0; i < currentCount; i++){
			if(vals[i].id == id){
				return &vals[i];
			}
		}
		
		return nullptr;
	}
	
	T* GetById(IDHandle<T> id) {
		return GetByIdNum(id.id);
	}

	void RemoveByIdNum(uint32 id){
		for(int i = 0; i < currentCount; i++){
			if(vals[i].id == id){
				vals[i].~T();
				memmove(&vals[i], &vals[i+1], sizeof(T) * (currentCount - i - 1));
				currentCount--;
				break;
			}
		}
	}

	void RemoveById(IDHandle<T> id) {
		RemoveByIdNum(id.id);
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
