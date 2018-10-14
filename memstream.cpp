#include "memstream.h"

#include <stdio.h>

void MemStream::EnsureCapacity(int newCapacity){
	if(newCapacity > capacity){
		void* newBase = malloc(newCapacity);
		
		int writeOffset = VOID_PTR_DIST(writeHead, readHead);
		MemCpy(newBase, readHead, writeOffset);
		
		free(base);
		base = newBase;
		readHead = base;
		writeHead = VOID_PTR_ADD(base, writeOffset);
	}
}

void MemStream::WriteString(const char* str){
	int length = StrLen(str);
	
	WriteString(str, length + 1);
}

void MemStream::WriteString(const char* str, int len){
	int newCapacity = capacity;
	int neededCapacity = VOID_PTR_DIST(writeHead, base) + len;
	while(newCapacity < neededCapacity){
		newCapacity = (newCapacity > 0) ? newCapacity * 2 : 16;
	}
	
	EnsureCapacity(newCapacity);
		
	MemCpy(writeHead, str, len);
	writeHead = VOID_PTR_ADD(writeHead, len);
}

void MemStream::ReadInFromFile(const char* fileName){
	FILE* fIn = fopen(fileName, "rb");
	
	if(fIn == NULL){
		return;
	}
	
	fseek(fIn, 0, SEEK_END);
	int fileSize = ftell(fIn);
	fseek(fIn, 0, SEEK_SET);
	
	if (fileSize > 0){
		capacity = fileSize;
		base = malloc(capacity);
		int bytesRead = (int)fread(base, 1, fileSize, fIn);
		ASSERT(bytesRead == fileSize);
		writeHead = VOID_PTR_ADD(base, fileSize);
		readHead = base;
	}
	else{
		// TODO: This shouldn't be a reset?
		base = nullptr;
		readHead = nullptr;
		writeHead = nullptr;
	}
	
	fclose(fIn);
}

#if defined(MEMSTREAM_TEST_MAIN)

struct TestStr{
	char name[5];
	int x;
	
	bool operator==(const TestStr& other){
		return StrEqualN(other.name, name, 5) && x == other.x;
	}
};

TestStr GetStruct(){
	TestStr c = {"ergg", -12345};
	
	return c;
}

CREATE_TEST_CASE("MemStream") {

	{
		MemStream str;
		
		str.Write(1322);
		str.Write(-235.03423f);
		str.Write(17);
		str.Write(12);
		str.Write(13);
		str.Write(14);
		
		ASSERT(str.Read<int>() == 1322);
		ASSERT(str.Read<float>() == -235.03423f);
		ASSERT(str.Read<int>() == 17);
		
		int arr[3];
		
		str.ReadArray(arr, 3);
		for(int i = 0; i < 3; i++){
			ASSERT(arr[i] == 12 + i);
		}
	}
	
	{
		MemStream str;
		
		str.Write(1422);
		ASSERT(str.Read<int>() == 1422);
		
		for(int i = 0; i < 24; i++){
			str.Write(GetStruct());
			ASSERT(str.Read<TestStr>() == GetStruct());
		}
	}
	
	{
		MemStream str;
		
		const int arrLength = 16;
		
		int arr[arrLength];
		for(int i = 0; i < arrLength; i++){
			arr[i] = i * i;
		}
		
		str.WriteArray(arr, arrLength);
		
		int otherArr[arrLength];
		
		str.ReadArray(otherArr, arrLength);
		
		for(int i = 0; i < arrLength; i++){
			ASSERT(arr[i] == otherArr[i]);
		}
	}
	
	{
		const char* testStr = "This is a test string. La la la#$^@$^@ l2  46246ga la la ";
		int testStrLen = StrLen(testStr);
		
		MemStream str;
		str.Write(-12345);
		str.WriteString(testStr);
		str.Write(54321);
		
		ASSERT(str.Read<int>() == -12345);
		
		str.WriteString(testStr);
		
		char buffer[512] = {0};
		
		str.ReadArray<char>(buffer, testStrLen + 1);
		
		ASSERT(StrEqual(buffer, testStr));
		ASSERT(StrLen(buffer) == testStrLen);
		
		ASSERT(str.Read<int>() == 54321);
		
		str.ReadArray<char>(buffer, testStrLen);
		
		ASSERT(StrEqualN(buffer, testStr, testStrLen));
	}
	
	{
		StringMap<int> testMap;
		
		testMap.Insert("ABC", 125);

		StringMap<int> testMap2;


		MemStream stream;

		stream.WriteStringMap(&testMap);
		stream.ReadStringMap(&testMap2);

		int val = -1;
		testMap2.LookUp("ABC", &val);
		ASSERT(val == 125);
	}

	{
		StringMap<int> testMap;

		testMap.Insert("ABC", 125);
		testMap.Insert("ABC", 126);
		testMap.Insert("G", 12);
		testMap.Insert("This", 125);
		testMap.Insert("Yo", 126);

		StringMap<int> testMap2;


		MemStream stream;

		stream.WriteStringMap(&testMap);
		stream.ReadStringMap(&testMap2);

		int val = -1;
		testMap2.LookUp("ABC", &val);
		ASSERT(val == 126);

		testMap2.LookUp("G", &val);
		ASSERT(val == 12);

		testMap2.LookUp("This", &val);
		ASSERT(val == 125);

		testMap2.LookUp("Yo", &val);
		ASSERT(val == 126);
	}

	{
		MemStream str2;

		{
			MemStream str;
			str.Write<int>(12);
			str.Write<int>(12);
			str.Write<int>(12);

			str2 = str;
		}

		ASSERT(str2.Read<int>() == 12);
		ASSERT(str2.Read<int>() == 12);
		ASSERT(str2.Read<int>() == 12);
		ASSERT(str2.GetLength() == 0);
	}

	{
		MemStream str;
		str.Write(12);
		str.Write(24);

		{
			MemStream str2 = str;
			ASSERT(str2.Read<int>() == 12);
			ASSERT(str2.Read<int>() == 24);
		}

		ASSERT(str.Read<int>() == 12);
		ASSERT(str.Read<int>() == 24);
	}
	
	{
		MemStream str;
		str.ReadInFromFile("dir2/empty_file.txt");
	}
	
	{
		MemStream str;
		str.ReadInFromFile("dir2/empty_file.txt");
		str.Write(22);
		ASSERT(str.Read<int>() == 22);
	}
	
	{
		MemStream str;
		str.ReadInFromFile("dir2/non_empty_file.txt");
	}
	
	{
		MemStream str;
		str.ReadInFromFile("dir2/non_empty_file.txt");
		str.Write(22);
		char arr[4] = {};
		str.ReadArray<char>(arr, 3);
		ASSERT(arr[0] == 'G');
		ASSERT(arr[1] == 'H');
		ASSERT(arr[2] == 'T');
		ASSERT(str.Read<int>() == 22);
	}

	return 0;
}

#endif

