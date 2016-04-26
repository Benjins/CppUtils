#include "strings.h"

#include <stdio.h>

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

// FML.
// cppcheck-suppress unmatchedSuppression
// cppcheck-suppress uninitMemberVar
// cppcheck-suppress uninitVar
String::String(const SubString& substr) 
	:  String(substr.start, substr.length) {
}

int String::GetRef() const{
	if(string == nullptr){
		return 0;
	}
	
	return *(short*)(string - 6);
}

int String::GetLength() const{
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
		ASSERT(*ref > 0);
		(*ref)--;
		
		if(*ref == 0){
			free(ref);
			string = nullptr;
		}
	}
}

void SubString::Retain(){
	(*ref)++;
}

void SubString::Release(){
	if (ref){
		ASSERT(start != nullptr);
		ASSERT(*ref > 0);
		(*ref)--;
		
		if(*ref == 0){
			free(ref);
			ref = nullptr;
			start = nullptr;
			length = 0;
		}
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
	
	unsigned int counter = 0;
	while(counter < len && *s1 == *s2 && *s1 != '\0' && *s2 != '\0'){
		s1++;
		s2++;
		counter++;
	}
	
	return counter == len || *s1 == *s2;
}

void MemCpy(void* dest, const void* src, int bytes){
	char* bDest = (char*)dest;
	char* bSrc  = (char*)src;
	
	for(int i = 0; i < bytes; i++){
		bDest[i] = bSrc[i];
	}
}

int Atoi(const char* str){
	ASSERT(str != nullptr);
	
	int val = 0;
	int sign = 1;
	if (*str == '-') {
		sign = -1;
		str++;
	}

	while(*str >= '0' && *str <= '9'){
		val = (val * 10) + (*str - '0');
		str++;
	}
	
	return val*sign;
}

int StrFind(const char* haystack, const char* needle) {
	if (!needle || !haystack) {
		return -1;
	}

	const char* originalHaystack = haystack;
	while (*haystack) {
		const char* hCursor = haystack;
		const char* nCursor = needle;

		while (*nCursor && *hCursor && *nCursor == *hCursor) {
			nCursor++;
			hCursor++;
		}

		if (!*nCursor) {
			return haystack - originalHaystack;
		}

		haystack++;
	}

	return -1;
}

inline bool CompareString(const String& a, const String& b){
	int length = a.GetLength();
	return (b.GetLength() == length) && StrEqualN(a.string, b.string, length);
}

inline bool CompareString(const String& a, const SubString& b){
	int length = a.GetLength();
	return (b.length == length) && StrEqualN(a.string, b.start, length);
}

inline bool CompareString(const SubString& a, const SubString& b){
	return (a.length == b.length) && StrEqualN(a.start, b.start, a.length);
}


SubString& SubString::operator=(const SubString& other){
	if (other.ref != ref){
		Release();
		
		start = other.start;
		length = other.length;
		ref = other.ref;
		
		Retain();
	}
	else if (other.start != start || other.length != length){
		start = other.start;
		length = other.length;
	}
	
	return *this;
}

bool SubString::operator==(const SubString& other) const{
	return CompareString(*this, other);
}

bool SubString::operator!=(const SubString& other) const{
	return !CompareString(*this, other);
}

String& String::operator=(const String& other){
	if (string != other.string) {
		Release();
		string = other.string;
		Retain();
	}
	
	return *this;
}

bool String::operator==(const String& other) const{
	return CompareString(*this, other);
}

bool String::operator!=(const String& other) const{
	return !CompareString(*this, other);
}

bool String::operator==(const SubString& other) const{
	return CompareString(*this, other);
}

bool String::operator!=(const SubString& other) const{
	return !CompareString(*this, other);
}

bool String::operator==(const char* other) const{
	int length = GetLength();
	return StrEqualN(string, other, length);
}

bool String::operator!=(const char* other) const{
	int length = GetLength();
	return !StrEqualN(string, other, length);
}

String ReadStringFromFile(const char* fileName) {
	String str;

	FILE* fIn = fopen(fileName, "rb");
	fseek(fIn, 0, SEEK_END);
	int fileLength = ftell(fIn);
	fseek(fIn, 0, SEEK_SET);

	str.SetSize(fileLength);
	fread(str.string, 1, fileLength, fIn);
	str.string[fileLength] = '\0';

	fclose(fIn);

	str.Retain();
	
	return str;
}

bool SubString::operator==(const String& other) const{
	return CompareString(other, *this);
}

bool SubString::operator!=(const String& other) const{
	return !CompareString(other, *this);
}

bool SubString::operator==(const char* other) const{
	return StrEqualN(start, other, length);
}

bool SubString::operator!=(const char* other) const{
	return !StrEqualN(start, other, length);
}

#if defined(STRINGS_TEST_MAIN)

int main(int argc, char** argv){
	
	BNS_UNUSED(argc);
	BNS_UNUSED(argv);
	
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
	
	ASSERT(StrFind("abs", "abs") == 0);
	ASSERT(StrFind("ababababababs", "abs") == 10);
	ASSERT(StrFind("abSSS", "abs") == -1);
	ASSERT(StrFind("abbb", "abbyyyaaabbnbbbbn") == -1);
	ASSERT(StrFind("J121241381835", "818") == 8);
	ASSERT(StrFind("J121241381835", nullptr) == -1);
	ASSERT(StrFind(nullptr, "J121241381835") == -1);
	ASSERT(StrFind(nullptr, nullptr) == -1);
	ASSERT(StrFind("ABCDABCD", "ABC") == 0);
	ASSERT(StrFind("ABHHHHHHHABABABAHBAHBAHAB", "ABB") == -1);

	#define CHECK_NUM(num) ASSERT(Atoi(#num) == num)
	
	CHECK_NUM(00);
	CHECK_NUM(10);
	CHECK_NUM(1);
	CHECK_NUM(12);
	CHECK_NUM(1442); 
	CHECK_NUM(1662);
	CHECK_NUM(1812);
	CHECK_NUM(0);
	CHECK_NUM(7);
	CHECK_NUM(343247);
	CHECK_NUM(99999);
	CHECK_NUM(900000);
	CHECK_NUM(100);
	CHECK_NUM(1003200);
	
	#undef CHECK_NUM
	
	ASSERT(Atoi("0123") == 123);
	ASSERT(Atoi("0123000") == 123000);
	
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
		StringStackBuffer<256> buf1("%s__123", "ABC");
		StringStackBuffer<256> buf2("ABC__%d", 123);
		StringStackBuffer<256> buf3("ABC__%d", 321);
		StringStackBuffer<64> buf4("ABC__%d", 123);
		
		ASSERT(buf1 == buf1);
		ASSERT(buf2 == buf2);
		ASSERT(buf3 == buf3);
		ASSERT(buf4 == buf4);
		
		ASSERT(!(buf1 != buf1));
		ASSERT(!(buf2 != buf2));
		ASSERT(!(buf3 != buf3));
		ASSERT(!(buf4 != buf4));
		
		ASSERT(buf1 == buf2);
		ASSERT(!(buf1 != buf2));
		ASSERT(buf2 != buf3);
		ASSERT(buf1 == buf4);
		ASSERT(buf2 == buf4);
	}
	
	{
		StringStackBuffer<256> buf1("%s__123", "ABC");
		StringStackBuffer<256> buf2("%s", "ABC");
		StringStackBuffer<256> buf3("%s", "ABC");
		
		buf2.Append("__123");
		buf3.AppendFormat("__%d", 123);
		
		ASSERT(buf1 == buf2);
		ASSERT(buf1 == buf3);
		ASSERT(buf2 == buf3);
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
		
		str1 = str1;
		str1 = str1;
	}
	
	{
		char str1B[] = "ABCDEFGHIJ";
		String str1 = str1B;
		String str2 = str1B;
		
		ASSERT(str1 == str1);
		ASSERT(!(str1 != str1));
		ASSERT(!(str1 != str1B));
		ASSERT(str1 == str1B);
		ASSERT(str1 == str2);
		ASSERT(str2 == str1B);
		
		str1B[0] = '#';
		ASSERT(str1 != str1B);
		ASSERT(str2 != str1B);
		
		ASSERT(str1 == str2);
	}
	
	{
		char str1B[] = "ABCDEFGHIJ";
		String str1 = str1B;
		String str2 = str1B;
		
		String str3 = str1;
		str2 = str1;
		str1 = str2;
		str3 = str2;
	}
	
	{
		char str1B[] = "ABCDEFGHIJ";
		String str1 = str1B;
		
		char str2B[] = "UWENNQW";
		String str2 = str2B;
		
		ASSERT(str1 != str2);
		
		String str3 = str1;
		String str4 = str2;
		
		ASSERT(str1 == str3);
		ASSERT(str2 == str4);
		ASSERT(str3 != str4);
		
		{
			String temp = str3;
			str3 = str4;
			str4 = temp;
		}
		
		ASSERT(str1 == str4);
		ASSERT(str2 == str3);
		ASSERT(str3 != str4);
	}
	
	{
		char str1B[] = "ABCDEFGHIJ";
		String str1 = str1B;
		
		char str2B[] = "@#$ABCDE";
		String str2 = str2B;
		
		SubString sub1 = str1.GetSubString(1, 3);
		SubString sub2 = str2.GetSubString(4, 3);
		
		ASSERT(sub1 == sub2);
		
		String str3 = "BCD";
		String str4 = str3;
		String str5 = "bde";
		
		ASSERT(sub1 == str3);
		ASSERT(sub1 == str4);
		ASSERT(sub1 != str5);
		
		ASSERT(str3 == sub1);
		ASSERT(str4 == sub1);
		ASSERT(str5 != sub1);
		
		ASSERT(sub2 == str3);
		ASSERT(sub2 == str4);
		ASSERT(sub2 != str5);
	}
	
	{
		char strVal[] = "SGJLSRJ";
		SubString substr;
		
		{
			String str1 = strVal;
			
			substr = str1.GetSubString(4, 2);
			
			ASSERT(substr == "SR");
		}
		
		ASSERT(substr == "SR");
		ASSERT(substr.GetRef() == 1);
		
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
		
		substr1 = substr2;
		substr1 = substr2;
		substr2 = substr1;
		
		ASSERT(substr1.GetRef() == 3);
		ASSERT(substr2.GetRef() == 3);
	}
	
	{
		char str1B[] = "ABCDEFGHIJ";
		String str1 = str1B;
		
		SubString substr1,substr2;
		
		substr1 = str1.GetSubString(3, 4);
		substr2 = str1.GetSubString(6, 2);
		
		ASSERT(substr1 != substr2);
		ASSERT(substr2 != substr1);
		
		ASSERT(substr1.GetRef() == 3);
		ASSERT(substr2.GetRef() == 3);
		
		{
			SubString temp = substr1;
			substr1 = substr2;
			substr2 = temp;
		}
		
		ASSERT(substr1 != substr2);
		ASSERT(substr2 != substr1);
		
		ASSERT(substr1.GetRef() == 3);
		ASSERT(substr2.GetRef() == 3);
		
		ASSERT(substr1 == "GH");
		ASSERT(substr2 == "DEFG");
		
		SubString substr3 = str1.GetSubString(6, 3);
		
		ASSERT(substr1 != substr3);
		ASSERT(substr2 != substr3);

		str1.Release();
		ASSERT(substr1.GetRef() == 3);
		ASSERT(substr2.GetRef() == 3);
		
		substr1 = substr3;
		substr2 = substr3;
		ASSERT(substr1.GetRef() == 3);
		ASSERT(substr2.GetRef() == 3);
		ASSERT(substr3.GetRef() == 3);
		
		ASSERT(substr1 == substr2);
		ASSERT(substr2 == substr3);
		
		SubString substr4,substr5;
		substr5 = substr4 = substr1;
		
		ASSERT(substr1.GetRef() == 5);
		ASSERT(substr4.GetRef() == 5);
		str1.Retain();
		ASSERT(str1.GetRef() == 6);
	}
	
	{
		String str1 = "BCDEFGH";
		
		SubString substr = str1.GetSubString(3, 2);
		
		str1.Release();
		
		ASSERT(substr.GetRef() == 1);
		str1.Retain();
		ASSERT(StrEqualN(substr.start, "EF", 2));
	}
	
	return 0;
}




#endif
