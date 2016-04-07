#ifndef STRINGS_H
#define STRINGS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "assert.h"

struct SubString;

int StrLen(const char* str);

void MemCpy(void* dest, const void* src, int bytes);

/*
Structure of string allocation:
 __________________________________
|Ref|Len|  String.......         |0|
|_2_|_4_|__________N_____________|1|

*/

template<int capacity>
struct StringStackBuffer{
	char buffer[capacity];
	int length;
	
	StringStackBuffer(){
		buffer[0] = '\0';
		length = 0;
	}
	
	StringStackBuffer(const char* format, ...){
		va_list varArgs;
		va_start(varArgs, format);
		vsnprintf(buffer, capacity, format, varArgs);
		va_end(varArgs);
	}
	
	void Append(const char* str){
		snprintf(&buffer[length], capacity - length, "%s", str);
	}
	
	void AppendFormat(const char* format, ...){
		va_list varArgs;
		va_start(varArgs, format);
		vsnprintf(&buffer[length], capacity - length, format, varArgs);
		va_end(varArgs);
	}
};

struct String{
	char* string;
	
	String(){
		string = nullptr;
	}
	
	String(int size){
		string = nullptr;
		SetSize(size);
	}
	
	String(const String& other){
		string = other.string;
		Retain();
	}
	
	String(const char* start){
		string = nullptr;
		int newSize = StrLen(start);
		SetSize(newSize);
		
		MemCpy(string, start, newSize);
	}
	
	~String(){
		Release();
	}
	
	void SetSize(int size);
	
	int GetRef();
	
	int GetLength();
	
	SubString GetSubString(int index, int length);
	
	void Retain();
	
	void Release();
};

struct SubString{
	short* ref;
	char* start;
	int length;
	
	SubString(){
		ref = nullptr;
		start = nullptr;
		length = 0;
	}
	
	SubString(const SubString& other){
		ref = other.ref;
		start = other.start;
		length = other.length;
		
		Retain();
	}
	
	~SubString(){
		Release();
	}
	
	int GetRef(){
		if(ref == nullptr){
			return 0;
		}
		
		return *ref;
	}
	
	void Retain();
	
	void Release();
};

struct ConstSubString{
	const char* start;
	int length;
	
	ConstSubString(){
		start = nullptr;
		length = 0;
	}
	
	ConstSubString(const char* str, int len){
		length = len;
		start = str;
	}
};

bool StrEqual(const char* s1, const char* s2);
bool StrEqualN(const char* s1, const char* s2, unsigned int len);

#endif