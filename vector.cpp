#include <stdio.h>

#include "vector.h"
#include "macros.h"

#if defined(VECTOR_TEST_MAIN)

//Used to track potential resource leaks in the vector test
int fakeAllocCount = 0;

struct Resource{
	int pad;
	
	Resource(){
		pad = 0;
		fakeAllocCount++;
	}
	
	Resource(const Resource& orig){
		pad = 0;
		fakeAllocCount++;
		BNS_UNUSED(orig);
	}

	Resource& operator=(const Resource& orig) {
		pad = 0;
		fakeAllocCount++;
		BNS_UNUSED(orig);

		return *this;
	}
	
	~Resource(){
		fakeAllocCount--;
	}
};

Vector<Resource> func1(){
	Vector<Resource> vals(3);
	vals.PushBack(Resource());
	vals.PushBack(Resource());
	vals.PushBack(Resource());

	return vals;
}

int main(int argc, char** argv){
	BNS_UNUSED(argc);
	BNS_UNUSED(argv);

	Vector<int> iVec(20);
	
	Vector<int> iVec2;
	iVec2.PushBack(34);
	iVec2.PushBack(33);
	iVec2.PushBack(32);
	iVec2.PushBack(31);
	
	ASSERT(iVec2.count == 4);
	ASSERT(iVec2.Get(0) == 34);
	ASSERT(iVec2.Get(1) == 33);
	ASSERT(iVec2.Get(2) == 32);
	ASSERT(iVec2.Get(3) == 31);
	
	iVec2.Insert(2, 37);
	ASSERT(iVec2.count == 5);
	ASSERT(iVec2.Get(0) == 34);
	ASSERT(iVec2.Get(1) == 33);
	ASSERT(iVec2.Get(2) == 37);
	ASSERT(iVec2.Get(3) == 32);
	ASSERT(iVec2.Get(4) == 31);
	
	Vector<int> iVec3 = iVec2;
	ASSERT(iVec3.count == 5);
	ASSERT(iVec3.Get(0) == 34);
	ASSERT(iVec3.Get(1) == 33);
	ASSERT(iVec3.Get(2) == 37);
	ASSERT(iVec3.Get(3) == 32);
	ASSERT(iVec3.Get(4) == 31);
	
	iVec3.EnsureCapacity(25);
	iVec3.PushBack(22);
	
	ASSERT(iVec2.count == 5);
	ASSERT(iVec3.count == 6);
	ASSERT(iVec3.Get(5) == 22);
	
	iVec3.Clear();
	ASSERT(iVec3.count == 0);
	
	Vector<int>(39).Destroy();
	
	Vector<int> iVec4;
	iVec4.PushBack(23);
	iVec4.PushBack(24);
	iVec4.PushBack(25);
	ASSERT(iVec4.count == 3);
	
	for(int i = 0; i < 3; i++){
		iVec4.PopBack();
		ASSERT(iVec4.count == 2 - i);
	}
	
	void* oldData = iVec4.data;
	iVec4.PushBack(23);
	iVec4.PushBack(24);
	iVec4.PushBack(25);
	ASSERT(iVec4.count == 3);
	//No re-allocation should happen
	ASSERT(iVec4.data == oldData);
	
	iVec4.Remove(1);
	ASSERT(iVec4.count == 2);
	ASSERT(iVec4.Get(0) == 23);
	ASSERT(iVec4.Get(1) == 25);
	
	const int testCount = 5000;
	Vector<int> iVec5;
	for(int i = 0; i < testCount; i++){
		iVec5.PushBack(i);
	}
	
	ASSERT(iVec5.count == testCount);
	
	Vector<int> iVec6 = iVec5;
	
	for(int i = 0; i < testCount; i++){
		iVec5.PopBack();
	}

	ASSERT(iVec5.count == 0);
	
	for(int i = 0; i < testCount; i++){
		iVec6.Remove(0);
	}
	
	ASSERT(iVec6.count == 0);
	
	ASSERT(fakeAllocCount == 0);
	
	{
		Vector<Resource> rVec(5);
	}
	
	ASSERT(fakeAllocCount == 0);
	
	{
		Vector<Resource> rVec;
		
		for(int i = 0; i < 20; i++){
			Resource r;
			rVec.PushBack(r);
		}
	}
	
	ASSERT(fakeAllocCount == 0);
	
	{
		Vector<Resource> rVec;
		
		for(int i = 0; i < 20; i++){
			Resource r;
			rVec.PushBack(r);
		}
		
		rVec.Clear();
		
		ASSERT(fakeAllocCount == 0);
	}
	
	{
		Vector<Resource> rVec;
		
		for(int i = 0; i < 20; i++){
			Resource r;
			rVec.PushBack(r);
		}
		
		rVec.Clear();
		
		Vector<Resource> rVecCpy = rVec;
	}
	
	ASSERT(fakeAllocCount == 0);
	
	{
		Vector<Resource> vals = func1();
		Vector<Resource> vals2 = func1();
		
		vals2.PushBack(Resource());
		
		vals = vals;
		vals2 = vals;
		Vector<Resource> vals3 = vals2;
		
		ASSERT(vals.data != vals2.data);
		ASSERT(vals3.data != vals2.data);
	}

	{
		Vector<int> iVec1(4);
		iVec1.PushBack(3);
		iVec1.PushBack(4);
		iVec1.PushBack(8);
		
		Vector<int> iVec2;
		iVec2.PushBack(1);
		iVec2.PushBack(2);
		iVec2.PushBack(3);
		
		Vector<int> iVec3 = iVec2;
		iVec2 = iVec3 = iVec2;
		
		iVec1 = iVec2;
		
		ASSERT(iVec1.count == 3);
		ASSERT(iVec2.count == 3);
		ASSERT(iVec3.count == 3);
		
		ASSERT(iVec1.data != iVec2.data);
		ASSERT(iVec3.data != iVec2.data);
		ASSERT(iVec1.data != iVec3.data);
		
		for(int i = 0; i < 3; i++){
			ASSERT(iVec1.Get(i) == iVec2.Get(i));
			ASSERT(iVec2.Get(i) == iVec3.Get(i));
		}
	}
	
	ASSERT(fakeAllocCount == 0);

	{
		Vector<Resource> rVec;

		for (int i = 0; i < 20; i++) {
			Resource r;
			rVec.PushBack(r);
		}

		Vector<Resource> rVec2;

		for (int i = 0; i < 5; i++) {
			Resource r;
			rVec2.PushBack(r);
		}

		rVec.InsertVector(4, rVec2);

		ASSERT(rVec.count == 25);

		rVec.InsertVector(4, rVec2);
		ASSERT(rVec.count == 30);

		rVec2.InsertVector(3, rVec);

		ASSERT(rVec.count == 30);
		ASSERT(rVec2.count == 35);
	}

	ASSERT(fakeAllocCount == 0);

	{
		Vector<int> iVec;
		iVec.PushBack(0);
		iVec.PushBack(2);
		iVec.PushBack(3);

		Vector<int> iVec2;
		iVec2.PushBack(1);
		iVec2.PushBack(4);
		iVec2.PushBack(5);

		iVec.InsertVector(1, iVec2);

		ASSERT(iVec.count == 6);
		ASSERT(iVec.data[0] == 0);
		ASSERT(iVec.data[1] == 1);
		ASSERT(iVec.data[2] == 4);
		ASSERT(iVec.data[3] == 5);
		ASSERT(iVec.data[4] == 2);
		ASSERT(iVec.data[5] == 3);
	}

	{
		Vector<int> iVec;
		iVec.PushBack(0);
		iVec.PushBack(2);
		iVec.PushBack(3);

		Vector<int> iVec2;
		for (int i = 0; i < 10; i++) {
			iVec2.PushBack(4);
		}

		iVec.InsertVector(2, iVec2);

		ASSERT(iVec.count == 13);
		ASSERT(iVec.data[0] == 0);
		ASSERT(iVec.data[1] == 2);
		for (int i = 2; i < 12; i++) {
			ASSERT(iVec.data[i] == 4);
		}
		ASSERT(iVec.data[12] == 3);
	}

	{
		Vector<int> iVec2;
		iVec2.PushBack(4);
		iVec2.PushBack(3);
		iVec2.PushBack(2);

		Vector<int> iVec;
		iVec.PushBack(7);

		iVec.InsertVector(1, iVec2);

		ASSERT(iVec.data[0] == 7);
		ASSERT(iVec.data[1] == 4);
		ASSERT(iVec.data[2] == 3);
		ASSERT(iVec.data[3] == 2);
	}

	{
		Vector<int> iVec2;
		iVec2.PushBack(4);
		iVec2.PushBack(3);
		iVec2.PushBack(2);
		
		Vector<int> iVec3 = iVec2.GetSubVector(0, 3);
		ASSERT(iVec3.data[0] == 4);
		ASSERT(iVec3.data[1] == 3);
		ASSERT(iVec3.data[2] == 2);

		Vector<int> iVec4 = iVec2.GetSubVector(1, 1);
		ASSERT(iVec4.data[0] == 3);
		ASSERT(iVec4.count== 1);
	}

	ASSERT(fakeAllocCount == 0);

	{
		Vector<Resource> rVec;

		for (int i = 0; i < 20; i++) {
			Resource r;
			rVec.PushBack(r);
		}

		Vector<Resource> rVec2 = rVec.GetSubVector(3, 14);
		Vector<Resource> rVec3 = rVec.GetSubVector(7, 9);
		Vector<Resource> rVec4 = rVec.GetSubVector(17, 2);

		BNS_UNUSED(rVec2);
		BNS_UNUSED(rVec3);
		BNS_UNUSED(rVec4);
	}

	ASSERT(fakeAllocCount == 0);

	return 0;
}

#endif
