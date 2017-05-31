
#if defined(DISC_UNION_TEST_MAIN)
#include <stdio.h>
#include <stdlib.h>

#include "disc_union.h"

#include "vector.h"
#include "strings.h"

struct MyClass{
	char name[16];
	int i;
	float f;
	
	Vector<int> it;
};

struct MyOtherClass{
	void* data;
	int g;
	float h;
	short header[6];
};


// Define a macro that takes a macro
#define DISC_LIST(mac) \
	mac(MyClass) \
	mac(MyOtherClass) \
	mac(String)

// This generates the actual struct body
DEFINE_DISCRIMINATED_UNION(MyUnion, DISC_LIST)

// We don't need to undef it, but it's a good idea
#undef DISC_LIST


int main(int argc, char** argv){
	
	MyClass mc;
	mc.it.PushBack(1);
	MyClass mc2;
	mc2.it.PushBack(2);
	MyOtherClass moc;

	MyUnion un;
	ASSERT(un.type == MyUnion::UE_None);
	ASSERT(!un.IsMyClass());
	ASSERT(!un.IsMyOtherClass());
	ASSERT(!un.IsString());
	
	un = mc;
	ASSERT(un.type == MyUnion::UE_MyClass);
	ASSERT(un.IsMyClass());
	ASSERT(!un.IsMyOtherClass());
	ASSERT(!un.IsString());

	un = moc;
	ASSERT(un.type == MyUnion::UE_MyOtherClass);
	ASSERT(!un.IsMyClass());
	ASSERT(un.IsMyOtherClass());
	ASSERT(!un.IsString());

	un = mc;
	ASSERT(un.type == MyUnion::UE_MyClass);
	ASSERT(un.IsMyClass());
	ASSERT(!un.IsMyOtherClass());
	ASSERT(!un.IsString());

	un = mc2;
	ASSERT(un.type == MyUnion::UE_MyClass);

	un = mc;
	ASSERT(un.type == MyUnion::UE_MyClass);
	
	String str1 = "LALA";
	ASSERT(str1.GetRef() == 1);
	un = str1;
	ASSERT(un.type == MyUnion::UE_String);
	ASSERT(un.AsString() == str1);
	ASSERT(str1.GetRef() == 2);

	un = mc2;
	ASSERT(un.type == MyUnion::UE_MyClass);
	ASSERT(str1.GetRef() == 1);

	un = str1;

	ASSERT(str1.GetRef() == 2);

	{
		MyUnion un2 = un;
		ASSERT(un2.type == MyUnion::UE_String);
		ASSERT(str1.GetRef() == 3);
		MyUnion un3;
		un3 = un2;
		ASSERT(un3.type == MyUnion::UE_String);
		ASSERT(str1.GetRef() == 4);
	}

	ASSERT(str1.GetRef() == 2);

	return 0;
}
#endif
