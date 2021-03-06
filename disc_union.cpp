
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

void DoSomething(const MyUnion& mu) {
	ASSERT(mu.IsString());
}

struct IntBox{
	int val;
};

struct CharBox{
	char val;
};

#define DISC_LIST(mac) \
	mac(IntBox)        \
	mac(CharBox)       \

DEFINE_DISCRIMINATED_UNION(BasicUnion1, DISC_LIST)

#undef DISC_LIST

static_assert(BNS_ALIGNOF(BasicUnion1) == 4, "Alignment of DISC_UNION (assumes int is 32-bit aligned");


#define DISC_LIST(mac) \
	mac(String)        \
	mac(IntBox)           \
	mac(SubString)

DEFINE_DISCRIMINATED_UNION(BasicUnion2, DISC_LIST)

#undef DISC_LIST

static_assert(BNS_ALIGNOF(BasicUnion2) == sizeof(int*), "Alignment of DISC_UNION (assumes pointers are word-aligned");


CREATE_TEST_CASE("Disc union basic usage"){
	
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

	{
		MyUnion un2 = str1;
		ASSERT(str1.GetRef() == 3);
		DoSomething(str1);
		ASSERT(str1.GetRef() == 3);
		ASSERT(un2.IsString());
		const MyUnion& un2_ref = un2;
		ASSERT(un2_ref.IsString());
		ASSERT(un2_ref.AsString().GetRef() == 3);
		un2 = MyUnion();
		ASSERT(!un2_ref.IsString());

		ASSERT(str1.GetRef() == 2);
		un2 = str1;
		ASSERT(str1.GetRef() == 3);
	}

	ASSERT(str1.GetRef() == 2);
	
	{
		MyUnion un2 = str1;
		ASSERT(str1.GetRef() == 3);
		if (auto* str = un2.MaybeAsString()) {
			ASSERT(str->string == str1.string);
		} else {
			ASSERT(false);
		}
	}
	
	ASSERT(str1.GetRef() == 2);

	return 0;
}
#endif
