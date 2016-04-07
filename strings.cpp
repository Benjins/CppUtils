#include "strings.h"

void String::SetSize(int size){
	Release();
	
	void* alloc = malloc(6 + size + 1);
	
	string = ((char*)alloc) + 6;
	string[0] = '\0';
	
	short* ref = (short*)alloc;
	int* length = (int*)(string - 4);
	
	*ref = 1;
	*length = size;
}

int String::GetRef(){
	if(string == nullptr){
		return 0;
	}
	
	return *(short*)(string - 6);
}

int String::GetLength(){
	if(string == nullptr){
		return 0;
	}
	
	return *(int*)(string - 4);
}

SubString String::GetSubString(int index, int length){
	ASSERT(string != nullptr);
	ASSERT(GetLength() >= index + length);
	
	SubString substr;
	substr.start = &string[index];
	substr.ref = (short*)(string - 6);
	substr.length = length;
	
	substr.Retain();
	
	return substr;
}

void String::Retain(){
	(*(short*)(string - 6))++;
}

void String::Release(){
	if (string != nullptr){
		short* ref = (short*)(string - 6);
		(*ref)--;
		
		if(*ref == 0){
			free(ref);
		}
		
		string = nullptr;
	}
}

void SubString::Retain(){
	(*ref)++;
}

void SubString::Release(){
	if (ref){
		ASSERT(start != nullptr);
		(*ref)--;
		
		if(*ref == 0){
			free(ref);
			
		}
		
		ref = nullptr;
		start = nullptr;
		length = 0;
	}
}

int StrLen(const char* str){
	int len = 0;
	
	while(str != nullptr && *str != '\0'){
		str++;
		len++;
	}
	
	return len;
}

bool StrEqual(const char* s1, const char* s2){
	if(s1 == s2){
		return true;
	}
	
	if(s1 == nullptr || s2 == nullptr){
		return false;
	}
	
	while(*s1 == *s2 && *s1 != '\0' && *s2 != '\0'){
		s1++;
		s2++;
	}
	
	return (*s1 == '\0') && (*s2 == '\0');
}

bool StrEqualN(const char* s1, const char* s2, unsigned int len){
	if(s1 == s2){
		return true;
	}
	
	if(s1 == nullptr || s2 == nullptr){
		return false;
	}
	
	int counter = 0;
	while(counter < len && *s1 == *s2 && *s1 != '\0' && *s2 != '\0'){
		s1++;
		s2++;
		counter++;
	}
	
	return counter == len;
}

void MemCpy(void* dest, const void* src, int bytes){
	char* bDest = (char*)dest;
	char* bSrc  = (char*)src;
	
	for(int i = 0; i < bytes; i++){
		bDest[i] = bSrc[i];
	}
}


#if defined(STRINGS_TEST_MAIN)

int main(int argc, char* argv){
	
	ASSERT(StrLen("") == 0);
	ASSERT(StrLen("abc") == 3);
	ASSERT(StrLen("abc  GGG") == 8);
	
	ASSERT(StrEqual("", ""));
	ASSERT(StrEqualN("A", "F", 0));
	ASSERT(StrEqualN(nullptr, nullptr, 4));
	ASSERT(StrEqual(nullptr, nullptr));
	ASSERT(StrEqual("abccc", "abccc"));
	ASSERT(!StrEqual("abccc", "abccca"));
	ASSERT(StrEqualN("abccc", "abccca", 5));
	ASSERT(StrEqualN("abcccA", "abccca", 5));
	ASSERT(!StrEqualN("abccc", "abccca", 6));
	ASSERT(!StrEqual("abc", "ABC"));
	
	char stkConst1[] = "!@#$%^";
	char stkConst2[] = "!@#$%^";
	
	ASSERT(StrEqual(stkConst1, stkConst2));
	
	{
		StringStackBuffer<256> buf("%s", "ABCEDRF");
		
		ASSERT(StrEqual(buf.buffer, "ABCEDRF"));
	}
	
	{
		StringStackBuffer<256> buf("%s++%d", "ABCEDRF", 13);
		
		ASSERT(StrEqual(buf.buffer, "ABCEDRF++13"));
	}
	
	{
		char str1B[] = "ABCDEFGHIJ";
		String str1 = str1B;
		
		str1B[0] = 'Z';

		ASSERT(str1.GetLength() == 10);
		ASSERT(str1.GetRef() == 1);
		ASSERT(str1.string[0] == 'A');
	}
	
	{
		char str1B[] = "ABCDEFGHIJ";
		String str1 = str1B;
		String str2 = str1;
		
		str1B[0] = 'Z';

		ASSERT(str1.GetLength() == 10);
		ASSERT(str1.GetRef() == 2);
		ASSERT(str1.string[0] == 'A');
		
		ASSERT(str2.GetLength() == 10);
		ASSERT(str2.GetRef() == 2);
		ASSERT(str2.string[0] == 'A');
		
		str2.string[2] = 'M';
		
		ASSERT(str1.string[2] == 'M');
	}
	
	{
		char str1B[] = "ABCDEFGHIJ";
		String str1 = str1B;
		str1.SetSize(20);
		str1.SetSize(20);
		String str2 = str1;
		str1.SetSize(20);
		str2.SetSize(20);
		
		str1.Release();
	}
	
	{
		char str1B[] = "ABCDEFGHIJ";
		String str1 = str1B;
		
		SubString substr1 = str1.GetSubString(0, 4);
		ASSERT(substr1.length = 4);
		ASSERT(StrEqualN(substr1.start, "ABCD", 4));
		
		SubString substr2 = str1.GetSubString(2, 4);
		ASSERT(substr2.length = 4);
		ASSERT(StrEqualN(substr2.start, "CDEF", 4));
		
		ASSERT(str1.GetRef() == 3);
	}
	
	{
		String str1 = "BCDEFGH";
		
		SubString substr = str1.GetSubString(3, 2);
		
		str1.Release();
		
		
		ASSERT(substr.GetRef() == 1);
		ASSERT(StrEqualN(substr.start, "EF", 2));
	}
	
	return 0;
}




#endif
