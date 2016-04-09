#include <string.h>
#include <new>

#include "stringmap.h"
#include "assert.h"

template <typename T>
void StringMap<T>::Assign(const StringMap& orig){
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

template <typename T>
void StringMap<T>::Destroy(){
	Clear();
	
	free(hashes);
	free(names);
	free(values);
}

template <typename T>
void StringMap<T>::Clear(){
	for(int i = 0; i < count; i++){
		names[i].~String();
		values[i].~T();
	}
}

template <typename T>
void StringMap<T>::EnsureCapacity(int newCapacity){
	if(capacity < newCapacity){
		T* newValues = (T*)malloc(newCapacity*sizeof(T));
		String* newNames = (String*)malloc(newCapacity*sizeof(String));
		Hash* newHashes = (Hash*)malloc(newCapacity*sizeof(Hash));
		
		for(int i = 0; i < count; i++){
			new (&newValues[i]) T();
		}
		
		for(int i = 0; i < count; i++){
			newHashes[i] = hashes[i];
		}
		
		for(int i = 0; i < count; i++){
			new (&newNames[i]) String(names[i]);
		}

		memcpy(newHashes, hashes, count * sizeof(Hash));
		memcpy(newNames, names, count * sizeof(String));
		memcpy(newValues, values, count * sizeof(T));
			
		Destroy();
		
		hashes = newHashes;
		values = newValues;
		names = newNames;
		
		capacity = newCapacity;
	}
}

template <typename T>
void StringMap<T>::InsertAtIndex(const String& name, Hash hash, const T& value, int index){
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

template <typename T>
void StringMap<T>::Insert(const String& name, const T& value){
	Hash hash = ComputeHash(name.string);
	
	// TODO: This could be faster with a binary search
	if (count == 0){
		InsertAtIndex(name, hash, value, 0);
	}
	else{
	
		int high = count - 1;
		for(int i = count - 1; i >= 0; i--){
			if(hashes[i] > hash){
				high = i;
			}
			else{
				break;
			}
		}
		
		if(hashes[high] == hash){
			bool found = false;
			for(int i = high; i >= 0; i--){
				if(names[i] == name){
					found = true;
					values[i] = value;
					break;
				}
				else if(hashes[i] < hash){
					break;
				}
			}
			
			if(!found){
				InsertAtIndex(name, hash, value, high + 1);
			}
		}
		else{
			InsertAtIndex(name, hash, value, high);
		}
	}
	
}

template <typename T>
bool StringMap<T>::LookUp(const String& name, T* out){
	Hash hash = ComputeHash(name.string);
	
	int high = count - 1;
	for(int i = count - 1; i >= 0; i--){
		if(hashes[i] == hash){
			high = i;
			break;
		}
		else if(hashes[i] < hash){
			break;
		}
	}
	
	if(hashes[high] == hash){
		bool found = false;
		for(int i = high; i >= 0; i--){
			if(names[i] == name){
				*out = values[i];
				return true;
			}
			else if(hashes[i] < hash){
				break;
			}
		}
	}
	
	
	return false;
}

#if defined(STRINGMAP_TEST_MAIN)

int resourceAlloc = 0;

struct Resource {
	int vals;

	Resource() {
		vals = 0;
		resourceAlloc++;
	}

	Resource(const Resource& other) {
		vals = other.vals;
		resourceAlloc++;
	}

	~Resource() {
		resourceAlloc--;
	}
};

int main(int argc, char** argv){
	BNS_UNUSED(argc);
	BNS_UNUSED(argv);
	
	{
		StringMap<int> phoneNumbers;
		phoneNumbers.Insert("Benji", 666);
		phoneNumbers.Insert("Daniel", 2445);
	}

	{
		StringMap<int> phoneNumbers;
		phoneNumbers.Insert("Benji", 666);
		phoneNumbers.Insert("Daniel", 2445);

		phoneNumbers.EnsureCapacity(10);
	}

	{
		StringMap<int> phoneNumbers;
		phoneNumbers.Insert("Benji", 666);
		phoneNumbers.Insert("Daniel", 2445);
		phoneNumbers.Insert("NotDaniel", ~2445);
	}

	{
		StringMap<int> phoneNumbers;

		ASSERT(phoneNumbers.count == 0);
		phoneNumbers.Insert("Benji", 666);
		ASSERT(phoneNumbers.count == 1);
		phoneNumbers.Insert("Daniel", 2445);
		ASSERT(phoneNumbers.count == 2);
		phoneNumbers.Insert("NotDaniel", ~2445);
		ASSERT(phoneNumbers.count == 3);

		int outVal = -11;
		ASSERT(!phoneNumbers.LookUp("Marek", &outVal));
		ASSERT(phoneNumbers.LookUp("Benji", &outVal) && outVal == 666);
		ASSERT(phoneNumbers.count == 3);
		phoneNumbers.Insert("Marek", 666);
		ASSERT(phoneNumbers.count == 4);
		outVal = -11;
		ASSERT(phoneNumbers.LookUp("Benji", &outVal) && outVal == 666);
		outVal = -11;
		ASSERT(phoneNumbers.LookUp("Marek", &outVal) && outVal == 666);
	}

	ASSERT(resourceAlloc == 0);

	{
		Resource res1;
		res1.vals = 1;
		
		Resource res2;
		res2.vals = 2;
		
		Resource res3;
		res3.vals = 3;

		Resource res4;
		res4.vals = 4;

		StringMap<Resource> resMap;

		resMap.Insert("Res1", res1);

		void* oldVals = resMap.values;

		resMap.Insert("Res1", res1);
		resMap.Insert("Res1", res2);
		resMap.Insert("Res1", res3);
		resMap.Insert("Res1", res4);

		ASSERT(resMap.count == 1);
		//We shouldn't have re-allocated
		ASSERT(resMap.values == oldVals);

		resMap.Insert("Res2", res2);
		resMap.Insert("Res3", res3);
		resMap.Insert("Res4", res4);

		ASSERT(resMap.count == 4);
	}

	ASSERT(resourceAlloc == 0);

	
	return 0;
}


#endif