#include "unicode.h"
#include "strings.h"
#include "macros.h"

U32String DecodeUTF8(const char* _start, int byteCount){
	// You know, you'd think static_cast would work.  But apparently not.
	const unsigned char* start = reinterpret_cast<const unsigned char*>(_start);
	U32String decoded = {};
	decoded.length = 0;
	decoded.start = (unsigned int*)malloc(byteCount*sizeof(unsigned int));
	
	unsigned char highBits[] = {0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};
	
	for (int i = 0; i < byteCount; ){
		if ((start[i] & 0x80) == 0){
			// High bit not set, just ascii
			decoded.start[decoded.length] = start[i];
			decoded.length++;
			i++;
		}
		else{
			bool foundBit = false;
			for (int j = 0; j < BNS_ARRAY_COUNT(highBits) - 1; j++){
				if ((start[i] & highBits[j + 1]) == highBits[j]){
					int codepoint = (start[i] & (~highBits[j + 1]));
					for (int k = 0; k < j + 1; k++){
						if ((start[i + 1 + k] & 0xC0) != 0x80){
							ASSERT_WARN("Subsequent byte in UTF-8 string with improper high two bits: 0x%X", start[i + 1 + k]);
						}
						codepoint = (codepoint << 6) | (start[i + 1 + k] & 0x3F);
					}
					
					decoded.start[decoded.length] = codepoint;
					decoded.length++;
					i += (j + 2);
					foundBit = true;
					break;
				}
			}
			
			ASSERT_MSG(foundBit, "Improper formatting on byte: 0x%X", start[i]);
		}
	}
	
	return decoded;
}

int GetUTF8Size(const U32String string){
	unsigned int upperLimits[] = {0x80, 0x800, 0x10000, 0x200000, 0x4000000, 0x80000000};

	int byteCount = 0;
	for (int i = 0; i < string.length; i++) {
		bool foundLimit = false;
		for (int j = 0; j < BNS_ARRAY_COUNT(upperLimits); j++) {
			if (string.start[i] < upperLimits[j]) {
				byteCount += (j + 1);
				foundLimit = true;
				break;
			}
		}

		ASSERT_MSG(foundLimit, "Unicode codepoint %d out of range, occur at position %d.", string.start[i], i);
	}

	return byteCount;
}

int EncodeUTF8(const U32String inString, unsigned char* outString){
	unsigned int upperLimits[] = { 0x800, 0x10000, 0x200000, 0x4000000, 0x80000000 };
	unsigned char highBits[] = { 0xC0, 0xE0, 0xF0, 0xF8};

	int bytesWritten = 0;
	for (int i = 0; i < inString.length; i++) {
		if (inString.start[i] < 0x80) {
			outString[bytesWritten] = inString.start[i];
			bytesWritten++;
		}
		else {
			bool foundLimit = false;
			int codepoint = inString.start[i];
			for (int j = 0; j < BNS_ARRAY_COUNT(upperLimits); j++) {
				if (codepoint < upperLimits[j]) {
					for (int k = j + 1; k > 0; k--) {
						outString[bytesWritten + k] = (codepoint & 0x3F) | 0x80;
						codepoint >>= 6;
					}

					outString[bytesWritten] = codepoint | highBits[j];

					bytesWritten += (j + 2);
					foundLimit = true;
					break;
				}
			}

			ASSERT_MSG(foundLimit, "Unicode codepoint %d out of range, occur at position %d.", inString.start[i], i);
		}
	}

	return bytesWritten;
}

int U32FindChar(const U32String string, int codepoint){
	for (int i = 0; i < string.length; i++) {
		if (string.start[i] == codepoint) {
			return i;
		}
	}

	return -1;
}

int U32StrFind(const U32String haystack, const U32String needle){
	int hIndex = 0, nIndex = 0;
	while (hIndex < haystack.length && nIndex < needle.length) {
		if (haystack.start[hIndex] == needle.start[nIndex]) {
			nIndex++;
		}
		else {
			hIndex -= nIndex;
			nIndex = 0;
		}

		hIndex++;
	}

	if (nIndex == needle.length) {
		return hIndex - nIndex;
	}
	else {
		return -1;
	}
}

U32String U32StrDup(const U32String string){
	U32String newStr;
	newStr.length = string.length;
	newStr.start = (unsigned int*)malloc((newStr.length + 1) * sizeof(unsigned int));
	
	MemCpy(newStr.start, string.start, newStr.length * sizeof(unsigned int));
	newStr.start[newStr.length] = 0;
	
	return newStr;
}

// Strips out any characters outside of 0-127 ascii range
void ConvertToAscii(const U32String string, char* outAscii){
	
}

// Search is inclusive for both low and high (i.e. ascii search would be 0, 127)
int CountCharactersInRange(const U32String string, int codepointLow, int codepointHigh){
	return -1;
}

void DeallocateU32String(U32String string){
	free(string.start);
}


#if defined(UNICODE_TEST_MAIN)

#include "filesys.h"

int main(int argc, char** argv){
	
	
	int fileLength = 0;
	unsigned char* fileBytes = ReadBinaryFile("unicode_test.txt", &fileLength);
	
	U32String utf32 = DecodeUTF8((char*)fileBytes, fileLength);

	int reEncodeSize = GetUTF8Size(utf32);

	unsigned char* reEncoded = (unsigned char*)malloc(reEncodeSize);
	int bytesWritten = EncodeUTF8(utf32, reEncoded);

	ASSERT(reEncodeSize == fileLength);

	int diff = memcmp(fileBytes, reEncoded, reEncodeSize);
	ASSERT(diff == 0);
	
	U32String utf32dup = U32StrDup(utf32);
	ASSERT(utf32dup.length == utf32.length);
	int dupDiff = memcmp(utf32dup.start, utf32.start, utf32.length);
	ASSERT(dupDiff == 0);

	// 0x6211 is the codepoint for the chinese character 'wo3', meaning me/I
	int locOfWo = U32FindChar(utf32, 0x6211);
	ASSERT(locOfWo == 3);

	FILE* unicodeOut = fopen("unicode__out.txt", "wb");
	fwrite(reEncoded, 1, bytesWritten, unicodeOut);
	fclose(unicodeOut);

	DeallocateU32String(utf32dup);
	DeallocateU32String(utf32);
	
	free(fileBytes);
	free(reEncoded);
	
	return 0;
}


#endif
