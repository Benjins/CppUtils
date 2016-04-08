#ifndef VECTOR_H
#define VECTOR_H

#pragma once

#include <new>
#include <string.h>

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

#endif