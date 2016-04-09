#ifndef STRINGMAP_H
#define STRINGMAP_H

#pragma once

#include "hash.h"
#include "strings.h"

template<typename T>
struct StringMap{
	Hash* hashes;
	String* names;
	T* values;
	
	int capacity;
	int count;

	StringMap(){
		hashes = nullptr;
		names = nullptr;
		values = nullptr;
		
		capacity = 0;
		count = 0;
	}
	
	StringMap(const StringMap& orig){
		capacity = 0;
		count = 0;
		
		hashes = nullptr;
		names = nullptr;
		values = nullptr;
		
		Assign(orig);
	}

	~StringMap() {
		Destroy();
	}
	
	void Assign(const StringMap& orig);
	
	void EnsureCapacity(int newCapacity);	
	
	void Destroy();
	
	void Clear();
	
	void Insert(const String& name, const T& value);
	
	bool LookUp(const String& name, T* out);
	
	//Internal use
	void InsertAtIndex(const String& name, Hash hash, const T& value, int index);
};


#endif