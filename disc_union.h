#ifndef DISCUNION_H
#define DISCUNION_H

#pragma once

#define DISC_ENUM(str) UE_ ## str,

#define DISC_TYPE_STRINGIFY(str) #str ,

#define DISC_FIELDS(str) char str##_data[sizeof(str)];

#define DISC_TEAR_DOWN(str) case UE_ ## str : { ((str*)(str ## _data))->~ str (); } break;

#define DISC_ASSGN(str) \
void Assign(const str & orig){ \
	if (type != UE_ ## str ){\
		TearDown();\
	}\
	if (type == UE_None){new(& str ## _data) str(orig); type = UE_ ## str;}\
	else {*(str *)(str ## _data) = orig;}\
}

#define DISC_ASSGN_OP(str) \
str & operator=(const str & orig){ \
	Assign(orig);\
	return *(str *) str ## _data;\
}

#define DISC_ASSIGN_FROM_ORIG(str) \
	 case UE_ ## str : { *(str *) str ## _data = *(str *) orig. str ## _data; } break;

#define DISC_AS_METHOD(str) \
str & As ## str (){     \
	return *(str *) str ## _data;  \
}

#define DISC_CONST_AS_METHOD(str) \
const str & As ## str () const{     \
	return *(const str *) str ## _data;  \
}

#define DISC_IS_METHOD(str) \
bool Is ## str () const {     \
	return type == UE_ ## str; \
}

#define DISC_CPY_CONSTRUCTOR(str) \
case UE_ ## str : { new (str ## _data) str ( *(str*) orig. str ## _data ); } break;

template<int a, int b>
struct BNSCompileMax{
	enum {
		value = a > b ? a : b
	};
};

template<size_t a, size_t b>
struct BNSCompileMaxSize{
	enum{
		value = a > b ? a : b
	};
};

static_assert(BNSCompileMax<3, 6>::value == 6, "Check BNSCompileMax");
static_assert(BNSCompileMaxSize<3, 6>::value == 6, "Check BNSCompileMax");

// TODO: This is in C++11, although there are fallbacks if need be
#define BNS_ALIGNOF(t) alignof(t)

#define DISC_UNION_ALIGNMAX_OP(type) BNSCompileMax<BNS_ALIGNOF(type),

#define DISC_UNION_ALIGNMAX_ED(type) >::value

#define DISC_UNION_GLUE_TOKS_(a, b) a ## b
#define DISC_UNION_GLUE_TOKS(a, b) DISC_UNION_GLUE_TOKS_(a, b)

#define DEFINE_DISCRIMINATED_UNION(name, macro) \
const char* DISC_UNION_GLUE_TOKS(name, _typeNames)[] = {\
	"None",\
	macro(DISC_TYPE_STRINGIFY)\
};\
struct name { \
	enum UnionEnum{ \
		UE_None,\
		macro(DISC_ENUM) \
		UE_CountPlus1, \
		UE_Count = UE_CountPlus1 - 1\
	}; \
	const char* GetCurrentTypeName(){return DISC_UNION_GLUE_TOKS(name, _typeNames)[type];}\
	union {\
		macro(DISC_FIELDS) \
	}; \
	name () {type = UE_None;}\
	template<typename T> \
	name(const T& param) { type = UE_None; Assign(param);  }\
	void operator= ( const name & orig ){ \
		if (type == orig.type) { \
			switch(type){ \
				case UE_None: { } break; \
				macro(DISC_ASSIGN_FROM_ORIG) \
				case UE_CountPlus1: { } break; \
			}\
		} \
		else { \
			if (type != UE_None) { TearDown(); } \
			new (this) name (orig); \
		} \
	} \
	name ( const name & orig ){ \
		type = orig.type;\
		switch(type){\
			case UE_None: { } break;\
			macro(DISC_CPY_CONSTRUCTOR)\
			case UE_CountPlus1: { } break;\
		}\
	}\
	macro(DISC_ASSGN)  \
	macro(DISC_ASSGN_OP) \
	macro(DISC_AS_METHOD) \
	macro(DISC_CONST_AS_METHOD) \
	macro(DISC_IS_METHOD) \
	void TearDown(){\
		switch (type){\
			case UE_None: { } break;\
			macro(DISC_TEAR_DOWN)\
			case UE_CountPlus1: { } break;\
		}\
		type = UE_None;\
	}\
	~ name(){\
		TearDown();\
	}\
	UnionEnum type;\
} __attribute__ ((aligned (\
	macro(DISC_UNION_ALIGNMAX_OP)\
	0\
	macro(DISC_UNION_ALIGNMAX_ED)\
)));

#endif
