#ifndef BNVM_H
#define BNVM_H

#include "vector.h"
#include "assert.h"
#include "stringmap.h"

typedef unsigned char byte;

enum Instruction{
	I_INVALIDI = -1,
	I_ADDI = 0,
	I_MULTI,
	I_DIVI,
	I_SUBI,
	I_NEGATEI,
	I_ADDF,
	I_MULTF,
	I_DIVF,
	I_SUBF,
	I_NEGATEF,
	I_BRANCH,
	I_BRANCHIFZERO,
	I_BRANCHIFNOTZERO,
	I_CALL,
	I_RETURN,
	I_PRINTI,
	I_READI,
	I_PRINTF,
	I_READF,
	I_LOADI,
	I_STOREI,
	I_LOADF,
	I_STOREF,
	I_INTLIT,
	I_FLTLIT,
	I_EQI,
	I_EQF,
	I_NEQI,
	I_NEQF,
	I_GTI,
	I_GTF,
	I_GTEI,
	I_GTEF,
	I_LTEI,
	I_LTEF,
	I_LTI,
	I_LTF,
};

#define DEFAULT_MAX_STACK_LIMIT 8192

struct MemStack{
	Vector<byte> stackMem;
	
	int limit;
	
	MemStack(){
		limit = DEFAULT_MAX_STACK_LIMIT;
		stackMem.EnsureCapacity(limit);
	}
	
	template<typename T>
	T* Access(int offset){
		ASSERT(stackMem.count + offset + sizeof(T) < limit);
		return (T*)&stackMem.data[stackMem.count + offset];
	}
	
	void Increment(int amt);
	void Decrement(int amt);
	void SetSize(int amt);
};

struct TempStack{
	Vector<byte> stackMem;
	
	int limit;
	
	TempStack(){
		limit = DEFAULT_MAX_STACK_LIMIT;
		stackMem.EnsureCapacity(limit);
	}
	
	template<typename T>
	void Push(const T& val){
		ASSERT(stackMem.count + sizeof(T) < limit);
		
		*(T*)(stackMem.data + stackMem.count) = val;
		stackMem.count += sizeof(T);
	}
	
	template<typename T>
	T Pop(){
		ASSERT(stackMem.count >= sizeof(T));
		
		stackMem.count -= sizeof(T);
		return *(T*)(stackMem.data + stackMem.count);
	}
};

struct BNVM{
	Vector<byte> code;
	int pc;
	
	StringMap<int> functionPointers;
	
	TempStack tempStack;
	TempStack callStack;
	
	MemStack varStack;
	
	int Execute(const char* funcName);
};


#endif
