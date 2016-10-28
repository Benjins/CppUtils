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
		data = (T*)malloc(other.capacity*sizeof(T));
		count = other.count;
		capacity = other.capacity;
		for(int i = 0; i < other.count; i++){
			new(&data[i]) T(other.data[i]);
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
	
	void EnsureCapacity(int newCapacity){
		if(newCapacity > capacity){
			T* newData = (T*)malloc(newCapacity*sizeof(T));
			BNS_MEMCPY(newData, data, count*sizeof(T));
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
		return data[count - 1];
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

	T& EmplaceBack(){
		if(count >= capacity){
			EnsureCapacity(capacity == 0 ? 2 : capacity * 2);
		}
		
		new(&data[count]) T();
		count++;
		
		return data[count - 1];
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

	void InsertVector(int index, const Vector<T>& vec) {
		InsertArray(index, vec.data, vec.count);
	}
	
	void InsertArray(int index, const T* elem, int arrCount) {
		while (count + arrCount >= capacity) {
			EnsureCapacity(capacity == 0 ? 2 : capacity * 2);
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
	
	Vector(int initialCapacity){
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
		BNS_MEMCPY(data, l.begin(), l.size() * sizeof(T));
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

#endif
