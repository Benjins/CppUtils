#ifndef MEMSTREAM_H
#define MEMSTREAM_H

#pragma once

#include "assert.h"
#include "strings.h"
#include "stringmap.h"

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

	MemStream(const MemStream& orig) {
		base = nullptr;
		readHead = nullptr;
		writeHead = nullptr;
		capacity = 0;

		*this = orig;
	}

	MemStream& operator=(const MemStream& orig) {
		int len = orig.GetLength();
		EnsureCapacity(len);
		writeHead = VOID_PTR_ADD(readHead, VOID_PTR_DIST(orig.writeHead, orig.readHead));
		MemCpy(readHead, orig.readHead, len);

		return *this;
	}
	
	int GetLength() const{
		return VOID_PTR_DIST(writeHead, readHead);
	}
	
	char* ReadStringInPlace(){
		void* oldReadHead = readHead;
		
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
	
	void ResetWhenClear() {
		ASSERT(readHead == writeHead);
		readHead = base;
		writeHead = base;
	}
	
	template<typename T>
	void WriteArray(const T* vals, int count){
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
		ASSERT(VOID_PTR_DIST(writeHead, readHead) >= (int)sizeof(T));
		
		void* oldReadHead = readHead;
		readHead = VOID_PTR_ADD(readHead, sizeof(T));
		
		return *(T*)oldReadHead;
	}
	
	template<typename T>
	void ReadArray(T* outVals, int count){
		ASSERT(readHead != nullptr);
		ASSERT(VOID_PTR_DIST(writeHead, readHead) >= (int)(sizeof(T) * count));
		
		MemCpy(outVals, readHead, sizeof(T) * count);
		readHead = VOID_PTR_ADD(readHead, (int)(sizeof(T) * count));
	}

	template<typename T>
	void ReadStringMap(StringMap<T>* map) {
		int entryCount = Read<int>();
		map->Clear();
		map->EnsureCapacity(entryCount);

		for (int i = 0; i < entryCount; i++) {
			T value = Read<T>();
			int varNameLen = Read<int>();
			char* varName = ReadStringInPlace();
			ASSERT(StrLen(varName) == varNameLen);

			map->Insert(varName, value);
		}
	}

	template<typename T>
	void WriteStringMap(const StringMap<T>* map) {
		Write(map->count);
		for (int i = 0; i < map->count; i++) {
			Write(map->values[i]);
			Write(map->names[i].GetLength());
			WriteString(map->names[i].string);
		}
	}
	
	~MemStream(){
		free(base);
		base = nullptr;
	}
};


#endif
