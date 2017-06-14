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

#define DEFINE_DISCRIMINATED_UNION(name, macro) \
const char* BNS_GLUE_TOKS(name, _typeNames)[] = {\
	"None",\
	macro(DISC_TYPE_STRINGIFY)\
};\
struct name { \
	typedef name _BNS_DiscriminatedUnion; \
	enum UnionEnum{ \
		UE_None,\
		macro(DISC_ENUM) \
		UE_CountPlus1, \
		UE_Count = UE_CountPlus1 - 1\
	}; \
	const char* GetCurrentTypeName(){return BNS_GLUE_TOKS(name, _typeNames)[type];}\
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
};

#endif