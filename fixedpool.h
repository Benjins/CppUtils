#ifndef FIXEDPOOL_H
#define FIXEDPOOL_H

#pragma once

#include "bitset.h"

struct FixedPool{
	int capacity;
	int allocSize;
	void* mem;

	BitSet allocTracker;

	FixedPool();

	// Setup(16, 4) would allocate 64 bytes, dished out in 16 byte increments.
	void Setup(int allocSize, int count);

	void* Allocate();
	void Deallocate(void* ptr);
};



#endif

