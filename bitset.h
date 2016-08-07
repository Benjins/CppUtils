#ifndef BITSET_H
#define BITSET_H

#pragma once

struct BitSet{
	int bytesAlloc;
	int* values;

	BitSet();
	// initialCap is in bits
	explicit BitSet(int initCap);
	BitSet(const BitSet& orig);

	~BitSet();

	int GetBitCapacity() const;
	void EnsureCapacity(int bitCount);

	void Clear();
	void SetBit(int bit, bool val);
	bool GetBit(int bit) const;
};

template<int CAP>
struct FixedBitSet{
	int values[(CAP + 31)/32];

	void SetBit(int bit);
	bool GetBit(int bit, bool val) const;
};

bool GetBitInBitSet(const int* values, int bit);
void SetBitInBitSet(int* values, int bit, bool val);

#endif

