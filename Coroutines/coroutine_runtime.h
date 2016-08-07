#ifndef COROUTINERUNTIME_H
#define COROUTINERUNTIME_H

#pragma once

enum struct CoroutineResult {
	CR_Yield,
	CR_Return
};

typedef CoroutineResult(CoroutineFunc)(void*);

struct CoroutineInstance {
	void* wrapperData;
	CoroutineFunc* wrapperFunc;

	bool Next() {
		return wrapperFunc(wrapperData) != CoroutineResult::CR_Return;
	}
};

#define START_CR(inst, name) name##_Data name##__LINE__##__data; CoroutineInstance inst = {&name##__LINE__##__data, name##_Func}
#define START_CR_ARGS(inst, name, ...) name##_Data name##__LINE__##__data( __VA_ARGS__ ); CoroutineInstance inst = {&name##__LINE__##__data, name##_Func}

#define BNS_YIELD()
#define BNS_YIELD_FROM(f) f()
#define BNS_YIELD_FROM_ARGS(f, ...) f(__VA_ARGS__)

#endif
