#ifndef STRINGS_H
#define STRINGS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "assert.h"

struct SubString;

int FindChar(const char* str, char c);

int StrLen(const char* str);

void MemCpy(void* dest, const void* src, int bytes);
void MemSet(void* dst, int val, int bytes);

bool StrEqual(const char* s1, const char* s2);
bool StrEqualN(const char* s1, const char* s2, unsigned int len);
int Atoi(const char* str);
float Atof(const char* str);

char* StrDup(const char* str);

int StrFind(const char* haystack, const char* needle);

/*
Structure of string allocation:
 _____________________________________
|Ref|Len|  String.......         |NULL|
|_4_|_4_|__________N_____________|_1__|

*/

template<int capacity>
struct StringStackBuffer{
	char buffer[capacity];
	int length;
	
	StringStackBuffer(){
		buffer[0] = '\0';
		length = 0;
	}

	template<int otherCapacity>
	bool operator==(const StringStackBuffer<otherCapacity>& other) const{
		if(other.length == length){
			return StrEqualN(buffer, other.buffer, length);
		}
		else{
			return false;
		}
	}
	
	template<int otherCapacity>
	bool operator!=(const StringStackBuffer<otherCapacity>& other) const{
		if(other.length == length){
			return !StrEqualN(buffer, other.buffer, length);
		}
		else{
			return true;
		}
	}
	
	StringStackBuffer(const char* format, ...){
		buffer[0] = '\0';
		length = 0;
		va_list varArgs;
		va_start(varArgs, format);
		length += vsnprintf(buffer, capacity, format, varArgs);
		va_end(varArgs);
	}
	
	void Append(const char* str){
		length += snprintf(&buffer[length], capacity - length, "%s", str);
	}
	
	void AppendFormat(const char* format, ...){
		va_list varArgs;
		va_start(varArgs, format);
		length += vsnprintf(&buffer[length], capacity - length, format, varArgs);
		va_end(varArgs);
	}
};

struct String{
	char* string;
	
	String(){
		string = nullptr;
	}
	
	explicit String(int size){
		string = nullptr;
		SetSize(size);
	}
	
	String(const String& other){
		string = other.string;
		if (string != nullptr) {
			Retain();
		}
	}
	
	String(const char* start){
		string = nullptr;
		int newSize = StrLen(start);
		SetSize(newSize);
		
		if (newSize > 0) {
			MemCpy(string, start, newSize);
			string[newSize] = '\0';
		}
	}

	String(const char* start, int length) {
		string = nullptr;
		SetSize(length);

		if (length) {
			MemCpy(string, start, length);
			string[length] = '\0';
		}
	}

	String(const SubString& substr);

	String& operator=(const String& other);
	bool operator==(const String& other) const;
	bool operator!=(const String& other) const;
	
	bool operator==(const SubString& other) const;
	bool operator!=(const SubString& other) const;
	
	bool operator==(const char* other) const;
	bool operator!=(const char* other) const;
	
	~String(){
		Release();
	}
	
	void SetSize(int size);
	
	int GetRef() const;
	
	int GetLength() const;
	
	SubString GetSubString(int index, int length);
	
	String Insert(const char* str, int index) const;
	String Insert(char c, int index) const;
	String Remove(int index) const;

	void Retain();
	
	void Release();
};

String ReadStringFromFile(const char* fileName);

struct SubString{
	int* ref;
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
		
		if (ref != nullptr) {
			Retain();
		}
	}
	
	~SubString(){
		Release();
	}
	
	SubString& operator=(const SubString& other);
	bool operator==(const SubString& other) const;
	bool operator!=(const SubString& other) const;
	
	bool operator==(const String& other) const;
	bool operator!=(const String& other) const;
	
	bool operator==(const char* other) const;
	bool operator!=(const char* other) const;
	
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

#define STATIC_TO_SUBSTRING(str) String(str).GetSubString(0, sizeof(str) - 1)

#endif
