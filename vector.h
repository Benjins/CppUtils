#ifndef VECTOR_H
#define VECTOR_H

#pragma once

#include <new>
#include <initializer_list>
#include <string.h>

#include "assert.h"
#include "macros.h"

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
	
	void Assign(const Vector& other){
		if (other.capacity > 0) {
			data = (T*)malloc(other.capacity*sizeof(T));
			count = other.count;
			capacity = other.capacity;
			for (int i = 0; i < other.count; i++) {
				new(&data[i]) T(other.data[i]);
			}
		}
		else {
			data = nullptr;
			count = 0;
			capacity = 0;
		}
	}
	
	Vector(const Vector& other){
		Assign(other);
	}
	
	Vector& operator=(const Vector& other){
		if(other.data != data){
			Destroy();
			Assign(other);
		}
		
		return *this;
	}
	
	void Resize(int size) {
		ASSERT(size >= 0);
		EnsureCapacity(size);
		if (count < size) {
			while (count < size) {
				new(&data[count]) T();
				count++;
			}
		}
		else if (count > size) {
			while (count > size) {
				count--;
				data[count].~T();
			}
		}
	}

	void EnsureCapacity(int newCapacity){
		if(newCapacity > capacity){
			T* newData = (T*)malloc(newCapacity*sizeof(T));
			if (data != nullptr) {
				BNS_MEMCPY(newData, data, count*sizeof(T));
				free(data);
			}
			
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

	void Swap(Vector& other) {
		int _capacity = capacity;
		int _count = count;
		T* _data = data;

		capacity = other.capacity;
		count = other.count;
		data = other.data;

		other.capacity = _capacity;
		other.count = _count;
		other.data = _data;
	}

	void PopBack(){
		ASSERT(count > 0);
		count--;
		data[count].~T();
	}

	T& Back() {
		ASSERT(count > 0);
		return data[count - 1];
	}

	const T& Back() const {
		ASSERT(count > 0);
		return data[count - 1];
	}
	
	void Remove(int index){
		ASSERT(index >= 0 && index < count);
		data[index].~T();
		memmove(&data[index], &data[index+1], sizeof(T)*(count - index - 1));
		count--;
	}

	// Lower-Inclusive, Upper-Exclusive: RemoveRange(5, 6) removes 1 element: 5
	// RemoveRange(4, 4) does nothing
	void RemoveRange(int lIdx, int hIdx) {
		ASSERT(lIdx >= 0 && lIdx <= count);
		ASSERT(hIdx >= 0 && hIdx <= count);
		if (lIdx < hIdx) {
			for (int i = lIdx; i < hIdx; i++) {
				data[i].~T();
			}
			memmove(&data[lIdx], &data[hIdx], sizeof(T)*(count - hIdx));
			count -= (hIdx - lIdx);
		}
	}
	
	void PushBack(const T& elem){
		if(count >= capacity){
			EnsureCapacity(capacity == 0 ? 2 : capacity * 2);
		}
		
		new(&data[count]) T(elem);
		count++;
	}

	T& EmplaceBack(){
		if(count >= capacity){
			EnsureCapacity(capacity == 0 ? 2 : capacity * 2);
		}
		
		new(&data[count]) T();
		count++;
		
		return data[count - 1];
	}
	
	void Insert(int index, const T& elem){
		ASSERT(index >= 0 && index <= count);
		if(count >= capacity){
			EnsureCapacity(capacity == 0 ? 2 : capacity * 2);
		}
		
		char* insertLoc = (char*)&data[index];
		for(int i = (sizeof(T) * (count - index)) - 1; i >= 0; i--){
			insertLoc[i + sizeof(T)] = insertLoc[i];
		}
		
		new(&data[index]) T(elem);
		count++;
	}

	void InsertVector(int index, const Vector<T>& vec) {
		InsertArray(index, vec.data, vec.count);
	}
	
	void InsertArray(int index, const T* elem, int arrCount) {
		if (count + arrCount >= capacity) {
			EnsureCapacity(count + arrCount);
		}

		memmove(&data[index + arrCount], &data[index], (count - index)*sizeof(T));

		for (int i = 0; i < arrCount; i++) {
			new(&data[index+i]) T(elem[i]);
		}

		count += arrCount;
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
	
	explicit Vector(int initialCapacity){
		count = 0;
		capacity = 0;
		data = nullptr;
		EnsureCapacity(initialCapacity);
	}
	
	Vector(std::initializer_list<T> l){
		count = 0;
		capacity = 0;
		data = nullptr;
		
		EnsureCapacity(l.size());
		for (int i = 0; i < l.size(); i++) {
			new(&data[i]) T(l.begin()[i]);
		}
		count = l.size();
	}
	
	void AppendList(std::initializer_list<T> l){
		InsertArray(count, l.begin(), l.size());
	}
	
	T& Get(int idx){
		ASSERT(idx >= 0 && idx < count);
		return data[idx];
	}

	Vector<T> GetSubVector(int idx, int length) const {
		ASSERT(idx >= 0 && idx + length <= count);

		Vector<T> subVec;
		subVec.EnsureCapacity(length);
		subVec.count = length;
		for (int i = 0; i < length; i++) {
			new(&subVec.data[i]) T(data[i + idx]);
		}

		return subVec;
	}
	
	const T& Get(int idx) const{
		ASSERT(idx >= 0 && idx < count);
		return data[idx];
	}
	
	~Vector(){
		Destroy();
	}
};

#define BNS_VEC_MAP(from, to, action) (to).Clear(); (to).EnsureCapacity((from).count); \
	for (int index = 0; index < (from).count; index++){ \
		auto& item = (from).Get(index);\
		(to).EmplaceBack() = action; \
	}

#define BNS_VEC_FOLDR(result, from, base, action) { \
	auto& acc = result; acc = base; \
	for (int index = 0; index < (from).count; index++){ \
		auto& item = (from).Get(index); \
		acc = action; \
	} \
}

#define BNS_VEC_FILTER(from, good, pred) (good).Clear(); (good).EnsureCapacity((from).count); \
	for (int index = 0; index < (from).count; index++){ \
		auto& item = (from).data[index]; \
		if (pred) { \
			(good).PushBack(item); \
		}\
	}

#define BNS_VEC_FOREACH_NAME(vec, varName) for (auto varName = &(vec).data[0]; varName != &(vec).data[(vec).count]; varName++)
#define BNS_VEC_FOREACH(vec) BNS_VEC_FOREACH_NAME(vec, ptr)

#define BNS_FOR_NAME(var, count) for (int var = 0; var < (count); var++)
#define BNS_FOR_I(count) BNS_FOR_NAME(i, count)
#define BNS_FOR_J(count) BNS_FOR_NAME(j, count)

#define BNS_ARRAY_FOR_NAME(var, arr) BNS_FOR_NAME(var, BNS_ARRAY_COUNT(arr))
#define BNS_ARRAY_FOR_I(arr) BNS_ARRAY_FOR_NAME(i, arr)
#define BNS_ARRAY_FOR_J(arr) BNS_ARRAY_FOR_NAME(j, arr)

#define BNS_VEC_FOR_NAME(var, vec) BNS_FOR_NAME(var, (vec).count)
#define BNS_VEC_FOR_I(vec) BNS_VEC_FOR_NAME(i, vec)
#define BNS_VEC_FOR_J(vec) BNS_VEC_FOR_NAME(j, vec)

#define BNS_VEC_DUMB_SORT(vec, pred) do {                   \
	BNS_FOR_I((vec).count) {                                \
		for (int j = i; j > 0; j--) {                       \
			auto l = (vec).data[j - 1];						\
			auto r = (vec).data[j ];						\
			if (!(pred)) {									\
				auto tmp = (vec).data[j];					\
				(vec).data[j] = (vec).data[j - 1];			\
				(vec).data[j - 1] = tmp;					\
			}												\
			else {											\
				break;										\
			}												\
		}													\
	}                                                       \
} while(0)

#endif
