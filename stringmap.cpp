#include "stringmap.h"

#if defined(STRINGMAP_TEST_MAIN)

int smSMResourceAlloc = 0;

struct SMResource {
	int vals;

	SMResource() {
		vals = 0;
		smSMResourceAlloc++;
	}

	SMResource(const SMResource& other) {
		vals = other.vals;
		smSMResourceAlloc++;
	}

	~SMResource() {
		smSMResourceAlloc--;
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

CREATE_TEST_CASE("String map basic") {

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
		ASSERT(!phoneNumbers.LookUp(String("Marek"), &outVal));
		ASSERT(phoneNumbers.LookUp("Benji", &outVal) && outVal == 666);
		outVal = -11;
		ASSERT(phoneNumbers.LookUp(String("Benji"), &outVal) && outVal == 666);
		ASSERT(phoneNumbers.count == 3);
		phoneNumbers.Insert("Marek", 666);
		ASSERT(phoneNumbers.count == 4);
		outVal = -11;
		ASSERT(phoneNumbers.LookUp("Benji", &outVal) && outVal == 666);
		outVal = -11;
		ASSERT(phoneNumbers.LookUp("Marek", &outVal) && outVal == 666);
		
		CheckIsValidStringMap(phoneNumbers);
	}

	ASSERT(smSMResourceAlloc == 0);

	{
		SMResource res1;
		res1.vals = 1;
		
		SMResource res2;
		res2.vals = 2;
		
		SMResource res3;
		res3.vals = 3;

		SMResource res4;
		res4.vals = 4;

		StringMap<SMResource> resMap;

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
		
		CheckIsValidStringMap(resMap);
	}

	ASSERT(smSMResourceAlloc == 0);

	{
		SMResource res1;
		res1.vals = 1;
		
		SMResource res2;
		res2.vals = 2;
		
		SMResource res3;
		res3.vals = 3;

		SMResource res4;
		res4.vals = 4;

		StringMap<SMResource> resMap;

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
		
		CheckIsValidStringMap(resMap);

		StringMap<SMResource> resMap2 = resMap;

		CheckIsValidStringMap(resMap2);

		StringMap<SMResource> resMap3;
		resMap3 = resMap2;

		CheckIsValidStringMap(resMap3);

		StringMap<SMResource> resMap4;
		StringMap<SMResource> resMap5;
		resMap4 = resMap5;
		resMap2 = resMap4;
		ASSERT(resMap2.count == 0);

		resMap5.Insert("ff", res1);
		resMap3 = resMap5;
		ASSERT(resMap3.count == 1);
	}

	ASSERT(smSMResourceAlloc == 0);

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