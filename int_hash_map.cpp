#include <stdio.h>

#include "int_hash_map.h"

#if defined(INT_HASH_MAP_TEST_MAIN)

CREATE_TEST_CASE("int hash map basic") {
	PositiveIntHashMap<int> table;

	BNS_FOR_I(1000 * 1000) {
		table.Put(i, i * 7);
	}

	BNS_FOR_I(1000 * 1000) {
		int val = -1;
		bool present = table.Get(i, &val);
		ASSERT(present);
		ASSERT(val == i * 7);
	}

	return 0;
}

CREATE_TEST_CASE("int hash map basic in-depth") {
	PositiveIntHashMap<int> table;

	BNS_FOR_I(10 * 1000) {
		table.Put(i, i * 7);

		BNS_FOR_J(i + 1) {
			int val = -1;
			bool present = table.Get(j, &val);
			ASSERT(present);
			ASSERT(val == j * 7);
		}
	}

	return 0;
}

CREATE_TEST_CASE("int hash map empty") {
	PositiveIntHashMap<int> table;
	BNS_FOR_I(1000) {
		int val = -1;
		bool present = table.Get(i, &val);
		ASSERT(!present);
		ASSERT(val == -1);
	}

	return 0;
}

CREATE_TEST_CASE("int hash map empty") {
	PositiveIntHashMap<int> table;
	table.Put(1000, 0);

	BNS_FOR_I(1000) {
		int val = -1;
		bool present = table.Get(i, &val);
		ASSERT(!present);
		ASSERT(val == -1);
	}

	return 0;
}

#endif
