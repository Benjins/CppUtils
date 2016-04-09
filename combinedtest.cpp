
#include "stringmap.h"
#include "vector.h"
#include "assert.h"


#if defined(COMBINEDTESTS_TEST_MAIN)


int main(int argc, char** argv){
	
	{
		Vector<StringMap<int>> mapVec;
		mapVec.EnsureCapacity(100);
		
		StringMap<int> intMap;
		intMap.Insert("This is me.", 1235);
		intMap.Insert("This is you.", 88);
		
		const int vecSize = 5;
		
		for(int i = 0; i < vecSize; i++){
			mapVec.PushBack(intMap);
		}
		
		ASSERT(mapVec.count == vecSize);
		
		for(int i = 0; i < vecSize; i++){
			mapVec.PopBack();
		}
		
		ASSERT(mapVec.count == 0);
		
		for(int i = 0; i < vecSize; i++){
			mapVec.PushBack(intMap);
		}
		
		for(int i = 0; i < vecSize; i++){
			ASSERT(mapVec.Get(i).count == 2);
			
			int outVal = -11;
			ASSERT(mapVec.Get(i).LookUp("This is me.", &outVal) && outVal == 1235);
			ASSERT(mapVec.Get(i).LookUp("This is you.", &outVal) && outVal == 88);
		}
		
		ASSERT(mapVec.count == vecSize);
	}
	
	{
		Vector<Vector<StringMap<Vector<int>>>> ridic;
		
		Vector<StringMap<Vector<int>>> ridicChild;
		
		StringMap<Vector<int>> ridicMap;
		
		Vector<int> thatThing;
		thatThing.PushBack(12);
		thatThing.PushBack(16);
		thatThing.PushBack(19);
		
		Vector<int> thisThing;
		thisThing.PushBack(7);
		thisThing.PushBack(3);
		thisThing.PushBack(5);
		
		ridicMap.Insert("thatThing", thatThing);
		ridicMap.Insert("thisThing", thisThing);
		ridicMap.Insert("yes", thisThing);
		ridicMap.Insert("no", thisThing);
		ridicMap.Insert("maybe", thatThing);
		
		StringMap<Vector<int>> ridicMap2;
		StringMap<Vector<int>> ridicMap3;
		
		ridicMap3.Insert("no_13", thisThing);
		ridicMap3.Insert("may@$^@$^@$be", thatThing);
		ridicMap3.Insert("may@$^@$^@$be", thisThing);
		
		ridicChild.PushBack(ridicMap);
		ridicChild.PushBack(ridicMap); // Duplication is meant
		ridicChild.PushBack(ridicMap2);
		ridicChild.PushBack(ridicMap3);
		
		ridic.PushBack(ridicChild);
		ridic.PushBack(ridicChild);
		ridic.PushBack(ridicChild);
		ridic.PushBack(ridicChild);
		ridic.PushBack(ridicChild);
		ridic.PushBack(ridicChild);
		ridic.PushBack(ridicChild);
		ridic.PushBack(ridicChild);
	}
	
	return 0;
}


#endif