#include <stdio.h>
#include <stdlib.h>
#include <new>

#include "assert.cpp"

template<typename T>
struct Ref{
	void* mem;
	int* ref;

	Ref(){
		mem = nullptr;
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
		mem = other.mem;
		ref = other.ref;
		Retain();
	}

	void EnsureValidity(){
		ASSERT(ref != nullptr && mem != nullptr);
	}

	T* Get(){
		EnsureValidity();
		return (T*)mem;
	}

	void Retain(){
		EnsureValidity();
		(*ref)++;
	}

	void Release(){
		if(mem != nullptr){
			EnsureValidity();
			ASSERT_MSG(*ref > 0, "Released too much: %d.", *ref);

			(*ref)--;

			if(*ref == 0){
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

#if defined(REF_TEST_MAIN)

struct HelloYou{
	int nice[24];
	char* stuff;

	HelloYou(){
		stuff = 0;
	}
};

struct MyStruct{
	char name[256];

	int thisIsMe;
	float thisIsYou;

	Ref<HelloYou> youPtr;

	MyStruct(){
		thisIsMe = 0;
		thisIsYou = 1.0f;
		youPtr.Allocate();
	}
};


Ref<MyStruct> getVal(){
	Ref<MyStruct> val;
	val.Allocate();
	return val;
}


int main(int argc, char** argv){
	Ref<MyStruct> myStr;
	myStr.Allocate();
	myStr.Retain();

	Ref<MyStruct> otherStr = myStr;

	myStr.Release();

	Ref<MyStruct> thing = getVal();

	RefArray<MyStruct> msArr;
	msArr.Allocate(20);

	Ref<MyStruct> msArrElem = msArr.Get(4);

	ASSERT(msArrElem.Get()->thisIsYou == 1.0f);

	Ref<MyStruct> msArrElemCpy = msArrElem;

	msArrElemCpy.Allocate();
	msArrElemCpy.Allocate();

	msArrElem.Allocate();

	Ref<MyStruct> msArrElem2 = msArr.Get(12);

	ASSERT(msArrElem2.Get()->thisIsYou == 1.0f);

	ASSERT(myStr.Get()->thisIsYou == 1.0f);

	return 0;
}

#endif
