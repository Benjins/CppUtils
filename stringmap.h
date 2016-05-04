#ifndef STRINGMAP_H
#define STRINGMAP_H

#pragma once

#include <string.h>
#include <new>

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

	StringMap& operator=(const StringMap& orig) {
		Assign(orig);
		return *this;
	}
	
	void Assign(const StringMap& orig){
		EnsureCapacity(orig.capacity);
		count = orig.count;
		
		for(int i = 0; i < count; i++){
			new (&values[i]) T(orig.values[i]);
		}
		
		for(int i = 0; i < count; i++){
			hashes[i] = orig.hashes[i];
		}
		
		for(int i = 0; i < count; i++){
			new (&names[i]) String(orig.names[i]);
		}
	}
	
	void EnsureCapacity(int newCapacity){
		if(capacity < newCapacity){
			T* newValues = (T*)malloc(newCapacity*sizeof(T));
			String* newNames = (String*)malloc(newCapacity*sizeof(String));
			Hash* newHashes = (Hash*)malloc(newCapacity*sizeof(Hash));
			
			memcpy(newHashes, hashes, count * sizeof(Hash));
			memcpy(newNames, names, count * sizeof(String));
			memcpy(newValues, values, count * sizeof(T));
				
			free(hashes);
			free(names);
			free(values);
			
			hashes = newHashes;
			values = newValues;
			names = newNames;
			
			capacity = newCapacity;
		}
	}	
	
	void Destroy(){
		Clear();
		
		free(hashes);
		free(names);
		free(values);
	}
	
	void Clear(){
		for(int i = 0; i < count; i++){
			names[i].~String();
			values[i].~T();
		}
	}
	
	void Insert(const String& name, const T& value){
		Hash hash = ComputeHash(name.string);
		
		// TODO: This could be faster with a binary search
		if (count == 0){
			InsertAtIndex(name, hash, value, 0);
		}
		else{
		
			int low = 0;
			for( ; low < count && hash > hashes[low]; low++){
			}

			if(low < count && hashes[low] == hash){
				bool found = false;
				for(int i = low; i < count; i++){
					if(name == names[i]){
						found = true;
						values[i] = value;
						break;
					}
					else if(hash < hashes[i]){
						break;
					}
				}
				
				if(!found){
					InsertAtIndex(name, hash, value, low);
				}
			}
			else{
				InsertAtIndex(name, hash, value, low);
			}
		}

	}

	bool LookUp(const String& name, T* out) const{
		Hash hash = ComputeHash(name.string);
		
		int low = 0;
		for ( ; low < count && hash > hashes[low]; low++) {
		}

		if(low < count && hashes[low] == hash){
			for(int i = low; i < count; i++){
				if(names[i] == name){
					*out = values[i];
					return true;
				}
				else if(hash < hashes[i]){
					break;
				}
			}
		}

		return false;
	}
	
	//Internal use
	void InsertAtIndex(const String& name, Hash hash, const T& value, int index){
		if(count >= capacity){
			EnsureCapacity((capacity > 0) ? (capacity * 2) : 2);
		}
		
		memmove(&hashes[index+1], &hashes[index], (count - index) * sizeof(Hash));
		memmove(&names[index+1], &names[index], (count - index) * sizeof(String));
		memmove(&values[index+1], &values[index], (count - index) * sizeof(T));
		
		new(&names[index]) String();
		new(&values[index]) T();

		hashes[index] = hash;
		names[index] = name;
		values[index] = value;
		
		count++;
	}
};


#endif
