#include <stdio.h>

#include "ref.h"


#if defined(REF_TEST_MAIN)

struct HelloYou{
	int nice[24];
	char* stuff;

	HelloYou(){
		stuff = 0;
		nice[0] = 0;
	}
};

struct MyStruct{
	char name[256];

	int thisIsMe;
	float thisIsYou;

	Ref<HelloYou> youPtr;

	MyStruct(){
		name[0] = '\0';
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

void func1(Ref<MyStruct> ms){
	ms.Retain();
	ms.Get()->thisIsMe = 21;
	ms.Release();
}

void func2(Ref<MyStruct>& ms){
	ms.Retain();
	ms.Get()->thisIsMe = 23;
	ms.Release();
}

int main(int argc, char** argv){
	BNS_UNUSED(argc);
	BNS_UNUSED(argv);

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
	
	{
		Ref<MyStruct> ms1;
		ms1.Allocate();
		ms1.Allocate();
		ms1.Allocate();
		ms1.Allocate();
		
		Ref<MyStruct> ms4;
		{
			Ref<MyStruct> ms2 = ms1;
			Ref<MyStruct> ms3 = ms2;
			ms4 = ms3;
		}
		
		ms1.Allocate();
	}

	{
		MyStruct msN;
		msN.thisIsMe = 13;
		Ref<MyStruct> ms1(msN);
		
		ASSERT(ms1.Get()->thisIsMe == 13);
		
		func1(ms1);
		ASSERT(ms1.Get()->thisIsMe == 21);
		
		func2(ms1);
		ASSERT(ms1.Get()->thisIsMe == 23);
		
		ms1.Allocate();
	}
	
	return 0;
}

#endif
