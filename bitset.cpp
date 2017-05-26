#include "bitset.h"
#include "strings.h"

#include "assert.h"

BitSet::BitSet(){
	bytesAlloc = 0;
	values = nullptr;
}

BitSet::BitSet(int initialCap){
	bytesAlloc = (initialCap + 7) / 8;
	if (bytesAlloc > 0){
		values = (int*)malloc(((bytesAlloc + 3) / 4) * 4);
	}
	else{
		values = nullptr;
	}
}

BitSet::BitSet(const BitSet& orig){
	bytesAlloc = orig.bytesAlloc;
	if (bytesAlloc > 0){
		values = (int*)malloc(((bytesAlloc + 3) / 4) * 4);
		MemCpy(values, orig.values, bytesAlloc);
	}
	else{
		values = nullptr;
	}
}

BitSet::~BitSet(){
	free(values);
	values = nullptr;
}

int BitSet::GetBitCapacity() const {
	return bytesAlloc * 8;
}

void BitSet::EnsureCapacity(int newCap){
	int newBytesAlloc = (newCap + 7) / 8;
	if (newBytesAlloc > bytesAlloc){
		int* newValues = (int*)malloc(((newBytesAlloc + 3) / 4) * 4);
		if (values != nullptr){
			MemCpy(newValues, values, newBytesAlloc);
			free(values);
		}
		values = newValues;
		bytesAlloc = newBytesAlloc;
	}
}

void BitSet::Clear(){
	ASSERT(values != nullptr)
	MemSet(values, 0, bytesAlloc);
}

bool BitSet::GetBit(int bit) const{
	ASSERT(bit / 8 < bytesAlloc);
	return GetBitInBitSet(values, bit);
}

void BitSet::SetBit(int bit, bool val){
	ASSERT(bit / 8 < bytesAlloc);
	SetBitInBitSet(values, bit, val);
}

void SetBitInBitSet(int* vals, int bit, bool val){
	int index = (bit / 32);
	int bitNum = (bit % 32);

	vals[index] = val ? (vals[index] | (1 << bitNum)) : (vals[index] & ~(1 << bitNum));
}

bool GetBitInBitSet(const int* vals, int bit){
	int index = (bit / 32);
	int bitNum = (bit % 32);

	return (vals[index] & (1 << bitNum)) != 0;
}
