#ifndef MEMSTREAM_H
#define MEMSTREAM_H

#pragma once

#include "assert.h"
#include "strings.h"

#define VOID_PTR_ADD(ptr, bytes) ((void*)(((char*)(ptr))+bytes))

#define VOID_PTR_DIST(p1,p2) ((int) ( ((size_t)(p1)) - ((size_t) (p2)) ))

struct MemStream{
	void* base;
	void* readHead;
	void* writeHead;
	
	int capacity;
	
	MemStream(){
		base = nullptr;
		readHead = nullptr;
		writeHead = nullptr;
		
		capacity = 0;
	}
	
	int GetLength(){
		return VOID_PTR_DIST(writeHead, readHead);
	}
	
	char* ReadStringInPlace(){
		void* oldReadHead = readHead;
		
		int len = 0;
		while(*(char*)readHead != '\0'){
			readHead = VOID_PTR_ADD(readHead, 1);
		}
		
		//Advance past the null byte
		readHead = VOID_PTR_ADD(readHead, 1);
		
		return (char*)oldReadHead;
	}
	
	void ReadInFromFile(const char* fileName);
	
	void EnsureCapacity(int newCapacity);
	
	void WriteString(const char* str);
	
	void WriteString(const char* str, int len);
	
	template<typename T>
	void Write(const T& val){
		int newCapacity = capacity;
		int neededCapacity = VOID_PTR_DIST(writeHead, base) + sizeof(T);
		while(newCapacity < neededCapacity){
			newCapacity = (newCapacity > 0) ? newCapacity * 2 : 16;
		}
		
		EnsureCapacity(newCapacity);
		
		MemCpy(writeHead, &val, sizeof(T));
		writeHead = VOID_PTR_ADD(writeHead, sizeof(T));
	}
	
	template<typename T>
	void WriteArray(T* vals, int count){
		int newCapacity = capacity;
		int neededCapacity = VOID_PTR_DIST(writeHead, base) + sizeof(T) * count;
		while(newCapacity < neededCapacity){
			newCapacity = (newCapacity > 0) ? newCapacity * 2 : 16;
		}
		
		EnsureCapacity(newCapacity);
		
		MemCpy(writeHead, vals, count * sizeof(T));
		writeHead = VOID_PTR_ADD(writeHead, count * sizeof(T));
	}
	
	template<typename T>
	T Read(){
		ASSERT(readHead != nullptr);
		ASSERT(VOID_PTR_DIST(writeHead, readHead) >= sizeof(T));
		
		void* oldReadHead = readHead;
		readHead = VOID_PTR_ADD(readHead, sizeof(T));
		
		return *(T*)oldReadHead;
	}
	
	template<typename T>
	void ReadArray(T* outVals, int count){
		ASSERT(readHead != nullptr);
		ASSERT(VOID_PTR_DIST(writeHead, readHead) >= sizeof(T) * count);
		
		MemCpy(outVals, readHead, sizeof(T) * count);
		readHead = VOID_PTR_ADD(readHead, sizeof(T) * count);
	}
	
	~MemStream(){
		free(base);
		base = nullptr;
	}
};


#endif
