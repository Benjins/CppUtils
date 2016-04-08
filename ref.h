#ifndef REF_H
#define REF_H

#pragma once

#include <stdlib.h>
#include <new>

#include "assert.h"

template<typename T>
struct Ref{
	void* mem;
	int* ref;

	Ref(){
		mem = nullptr;
		ref = nullptr;
	}

	Ref(const T& init){
		void* alloc = malloc(sizeof(T)+sizeof(int));
		ref = (int*)alloc;
		*ref = 0;
		mem = new(ref+1) T(init);

		Retain();
	}

	void Allocate(){
		Release();

		void* alloc = malloc(sizeof(T)+sizeof(int));
		ref = (int*)alloc;
		*ref = 0;
		mem = new(ref+1) T();

		Retain();
	}

	Ref(const Ref& other){
		// cppcheck-suppress copyCtorPointerCopying
		mem = other.mem;
		ref = other.ref;
		Retain();
	}

	Ref& operator=(const Ref& other){
		if(other.mem != nullptr){
			other.Retain();
		}
		
		Release();
		
		mem = other.mem;
		ref = other.ref;
		
		return *this;
	}

	void EnsureValidity() const{
		ASSERT(ref != nullptr && mem != nullptr);
	}

	T* Get(){
		EnsureValidity();
		return (T*)mem;
	}

	void Retain() const{
		EnsureValidity();
		(*ref)++;
	}

	void Release(){
		if(mem != nullptr){
			EnsureValidity();
			ASSERT_MSG(*ref > 0, "Released too much: %d.", *ref);

			(*ref)--;

			if(*ref == 0){
				ASSERT_MSG(&ref[1] == mem, "%s", "Ref taken from array is being destroyed on its own.");
				((T*)mem)->~T();
				free(ref);
				mem = nullptr;
			}
		}
	}

	~Ref(){
		Release();
	}

};

template<typename T>
struct RefArray{
	void* mem;
	int count;

	RefArray(){
		mem = nullptr;
		count = 0;
	}

	void Allocate(int _count){
		ReleaseAll();
		count = _count;
		void* alloc = malloc(count*(sizeof(T) + sizeof(int)));

		int* refs = (int*)alloc;
		mem = refs + count;

		for(int i = 0; i < count; i++){
			refs[i] = 1;
		}

		T* vals = (T*)mem;
		vals = new(vals) T[count];
	}

private:
	//Currently we don't support multiple arrays with refs to the same elements.
	//Shouldn't be too hard though.
	RefArray(const RefArray& ){
	}
public:

	Ref<T> Get(int index){
		if(index < 0 || index >= count){
			printf("\nError: Access array at index (%d) out of bounds (0, %d).\n", index, count);
		}


		int* refs = ((int*)mem) - count;
		T* vals = (T*)mem;

		Ref<T> val;
		val.mem = &vals[index];
		val.ref = &refs[index];

		//Since we're directly setting the ref/mem in val, we need to manually increment ref count.
		val.Retain();

		return val;
	}

	void ReleaseAll(){
		if(mem != nullptr){
			int* refs = ((int*)mem) - count;
			T* vals = (T*)mem;
			for(int i = 0; i < count; i++){
				ASSERT(refs[i] == 1);
				vals[i].~T();
			}

			free(((int*)mem) - count);
		}
	}

	~RefArray(){
		ReleaseAll();
	}
};

#endif
