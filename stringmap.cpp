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
	
		int low = 0;
		for( ; low < count && hash > hashes[low]; low++){
		}
		
		if(hashes[low] == hash){
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

template <typename T>
bool StringMap<T>::LookUp(const String& name, T* out){
	Hash hash = ComputeHash(name.string);
	
	int low = 0;
	for ( ; low < count && hash > hashes[low]; low++) {
	}
	
	if(hashes[low] == hash){
		bool found = false;
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

template<typename T>
void CheckIsValidStringMap(const StringMap<T>& map){
	for(int i = 0; i < map.count; i++){
		if(i < map.count - 1){
			ASSERT(map.hashes[i] <= map.hashes[i+1]);
		}
		
		ASSERT(ComputeHash(map.names[i].string) == map.hashes[i]);
	}
}

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
		
		CheckIsValidStringMap(phoneNumbers);
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
		//resMap.Insert("Res1", res2);
		//resMap.Insert("Res1", res3);
		//resMap.Insert("Res1", res4);

		ASSERT(resMap.count == 1);
		//We shouldn't have re-allocated
		ASSERT(resMap.values == oldVals);

		resMap.Insert("Res2", res2);
		resMap.Insert("Res3", res3);
		resMap.Insert("Res4", res4);

		ASSERT(resMap.count == 4);
		
		CheckIsValidStringMap(resMap);
	}

	ASSERT(resourceAlloc == 0);

	{
		const char* str1 = "5\"\"\"";
		const char* str2 = "a0p2";
		
		ASSERT_MSG(ComputeHash(str1) == ComputeHash(str2), "This test requires '%s' and '%s' to have the same hashes.", str1, str2);
		
		StringMap<int> hashMap;
		hashMap.Insert(str1, 1234);
		hashMap.Insert(str2, 4321);
		
		ASSERT(hashMap.count == 2);
		
		CheckIsValidStringMap(hashMap);
		
		int outVal = -11;
		ASSERT(hashMap.LookUp(str1, &outVal) && outVal == 1234);
		ASSERT(hashMap.LookUp(str2, &outVal) && outVal == 4321);
	}
	
	{
		const char* str1 = "5\"\"\"";
		const char* str2 = "a0p2";
		
		ASSERT_MSG(ComputeHash(str1) == ComputeHash(str2), "This test requires '%s' and '%s' to have the same hashes.", str1, str2);
		
		StringMap<int> hashMap;
		hashMap.Insert("", 77);
		hashMap.Insert("This", 66);
		hashMap.Insert(str1, 1234);
		hashMap.Insert(str2, 4321);
		
		ASSERT(hashMap.count == 4);
		
		CheckIsValidStringMap(hashMap);
		
		int outVal = -11;
		ASSERT(hashMap.LookUp(str1, &outVal) && outVal == 1234);
		ASSERT(hashMap.LookUp(str2, &outVal) && outVal == 4321);
		ASSERT(hashMap.LookUp("", &outVal) && outVal == 77);
		ASSERT(hashMap.LookUp("This", &outVal) && outVal == 66);
	}
	
	
	{
		const char* str1 = "5\"\"\"";
		const char* str2 = "a0p2";
		
		ASSERT_MSG(ComputeHash(str1) == ComputeHash(str2), "This test requires '%s' and '%s' to have the same hashes.", str1, str2);
		
		StringMap<int> hashMap;
		hashMap.Insert(str1, 1234);
		hashMap.Insert(str1, 1234);
		hashMap.Insert(str1, 1234);
		hashMap.Insert(str1, 1234);
		hashMap.Insert(str2, 4321);
		hashMap.Insert(str1, 4321);
		hashMap.Insert(str2, 777);
		
		ASSERT(hashMap.count == 2);
		
		hashMap.Insert(str1, 55);
		
		CheckIsValidStringMap(hashMap);
		
		int outVal = -11;
		ASSERT(hashMap.LookUp(str1, &outVal) && outVal == 55);
		ASSERT(hashMap.LookUp(str2, &outVal) && outVal == 777);
	}
	
	{
		const char* str1 = "5\"\"\"";
		const char* str2 = "a0p2";

		ASSERT_MSG(ComputeHash(str1) == ComputeHash(str2), "This test requires '%s' and '%s' to have the same hashes.", str1, str2);

		StringMap<int> hashMap;

		hashMap.Insert(str1, 12);
		int outVal = -11;
		ASSERT(!hashMap.LookUp(str2, &outVal));
		ASSERT(hashMap.LookUp(str1, &outVal) && outVal == 12);
		hashMap.Insert(str2, 54);

		outVal = -11;
		ASSERT(hashMap.LookUp(str1, &outVal) && outVal == 12);
		ASSERT(hashMap.LookUp(str2, &outVal) && outVal == 54);
	}

	return 0;
}


#endif