#ifndef INT_HASH_MAP_H_
#define INT_HASH_MAP_H_

#pragma once

#define FIB_HASH_CONSTANT 11400714819323198485llu

// Can only store positive integer keys...don't ask...
// -1 used as marker for not-in-use slots
// Dumb, simple linear probing table
template<typename _Value>
struct PositiveIntHashMap {
	int* keys = nullptr;
	_Value* vals = nullptr;

	int maxKVPairCount = 0;
	int currentOccupancy = 0;
	float maxDesiredLoad = 0.5f;

	void ReHashtoSize(int nextSize) {
		const int oneMinusNewSlotCount = nextSize - 1;
		ASSERT_MSG((nextSize & oneMinusNewSlotCount) == 0, "ReHashtoSize must be called w/ powers of 2!!");

		int* newKeys = new int[nextSize];
		for (int i = 0; i < nextSize; i++) {
			newKeys[i] = -1;
		}

		_Value* newVals = new _Value[nextSize];

		for (int i = 0; i < maxKVPairCount; i++) {
			if (keys[i] >= 0) {
				int newSlot = GetFreeSlotForKeyInternal(keys[i], oneMinusNewSlotCount, newKeys);
				newKeys[newSlot] = keys[i];
				newVals[newSlot] = vals[i];
			}
		}

		delete[] keys;
		delete[] vals;

		keys = newKeys;
		vals = newVals;
		maxKVPairCount = nextSize;
	}

	~PositiveIntHashMap() {
		delete[] keys;
		delete[] vals;
	}

	void ReHashToNextSize() {
		const int nextSize = (maxKVPairCount == 0) ? 16 : (maxKVPairCount * 2);
		ReHashtoSize(nextSize);
	}

	int GetFreeSlotForKeyInternal(int key, int oneMinusSlotCount, const int* keys) {
		int newSlot = (int)((key * FIB_HASH_CONSTANT) & oneMinusSlotCount);
		int originalNewSlot = newSlot;
		while (keys[newSlot] != -1) {
			newSlot = (newSlot + 1) & oneMinusSlotCount;

			if (originalNewSlot == newSlot) {
				ASSERT(false);
			}
		}

		return newSlot;
	}

	void Put(int key, const _Value& val) {
		if (currentOccupancy >= (int)(maxDesiredLoad * maxKVPairCount)) {
			ReHashToNextSize();
		}

		int slot = GetFreeSlotForKeyInternal(key, maxKVPairCount - 1, keys);
		ASSERT(slot >= 0 && slot < maxKVPairCount);
		keys[slot] = key;
		vals[slot] = val;

		currentOccupancy++;
	}

	bool Get(int key, _Value* outVal) {
		if (maxKVPairCount == 0) { return false; }

		const int oneMinusSlotCount = maxKVPairCount - 1;
		int startSlot = (int)((key * FIB_HASH_CONSTANT) & oneMinusSlotCount);
		for (int iter = 0; iter < maxKVPairCount; iter++) {
			if (keys[startSlot] == -1) {
				return false;
			}
			else if (keys[startSlot] == key) {
				*outVal = vals[startSlot];
				return true;
			}
		}

		return false;
	}

	// TODO: Lol
	//void Erase(int key) {
	//
	//}


};


#endif
