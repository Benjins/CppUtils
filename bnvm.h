#ifndef BNVM_H
#define BNVM_H

#include "vector.h"
#include "assert.h"
#include "stringmap.h"
#include "memstream.h"

typedef unsigned char byte;

enum Instruction{
	I_INVALIDI = -1,
	I_NOOP = 0,
	I_ADDI,
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
	I_ITOF,
	I_EXTERNF,
	I_LOADG,
	I_STOREG,
	I_LINE,
	I_PRINTS,
	I_STRINGLIT,
	I_INTERNSTRTOEXTERN
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
		ASSERT(stackMem.count + offset + (int)sizeof(T) < limit);
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
		ASSERT(stackMem.count + (int)sizeof(T) < limit);
		
		*(T*)(stackMem.data + stackMem.count) = val;
		stackMem.count += sizeof(T);
	}
	
	template<typename T>
	T Pop(){
		ASSERT(stackMem.count >= (int)sizeof(T));
		
		stackMem.count -= sizeof(T);
		return *(T*)(stackMem.data + stackMem.count);
	}
};

typedef void (ExternFunc)(TempStack*);

struct BNVM;

enum BNVMDebugState {
	DS_Continue,
	DS_StepNext
};

enum BNVMReturnReason {
	RR_Done,
	RR_Break
};

struct BNVMInstance {
	StringMap<int> globalVarRegs;
	int globalVarSize;
	int pc;

	BNVMDebugState debugState;

	BNVM* vm;

	TempStack tempStack;
	TempStack callStack;

	MemStack varStack;

	int currentFileLine;

	BNVMInstance() {
		vm = nullptr;
		pc = 0;
		debugState = DS_Continue;
		currentFileLine = -1;
		globalVarSize = 0;
	}

	BNVMReturnReason Execute(const char* funcName);
	BNVMReturnReason ExecuteInternal();
	BNVMReturnReason ExecuteInternalFunc(const char* funcName);

	template<typename T>
	T GetGlobalVariableValue(const char* name) {
		int globalVarOffset = -1;
		globalVarRegs.LookUp(name, &globalVarOffset);
		T* globalVarLocation = (T*)&varStack.stackMem.data[globalVarOffset];

		return *globalVarLocation;
	}

	template<typename T>
	T GetGlobalVariableValueByOffset(int offset) {
		T* globalVarLocation = (T*)&varStack.stackMem.data[offset];

		return *globalVarLocation;
	}

	template<typename T>
	void SetGlobalVariableValue(const char* name, T val) {
		int globalVarOffset = -1;
		globalVarRegs.LookUp(name, &globalVarOffset);
		T* globalVarLocation = (T*)&varStack.stackMem.data[globalVarOffset];
		*globalVarLocation = val;
	}

	// TODO: Combine reason with return type?
	template<typename Arg, typename Ret>
	Ret ExecuteTyped(const char* funcName, const Arg& arg) {
		Arg swizzled;
		const int* cursor = (const int*)(&arg);
		int* swizCursor = (int*)&swizzled;

		int wordCount = (sizeof(Arg) + 3) / 4;
		for (int i = 0; i < wordCount; i++) {
			swizCursor[i] = cursor[wordCount - 1 - i];
		}

		tempStack.Push<Arg>(swizzled);
		varStack.Increment(globalVarSize);
		BNVMReturnReason reason = ExecuteInternalFunc(funcName);

		ASSERT(tempStack.stackMem.count == sizeof(Ret));
		return tempStack.Pop<Ret>();
	}

	template<typename Arg>
	BNVMReturnReason ExecuteTyped(const char* funcName, const Arg& arg) {
		Arg swizzled;
		const int* cursor = (const int*)(&arg);
		int* swizCursor = (int*)&swizzled;

		int wordCount = (sizeof(Arg) + 3) / 4;
		for (int i = 0; i < wordCount; i++) {
			swizCursor[i] = cursor[wordCount - 1 - i];
		}

		tempStack.Push<Arg>(swizzled);
		varStack.Increment(globalVarSize);
		BNVMReturnReason reason = ExecuteInternalFunc(funcName);

		ASSERT(tempStack.stackMem.count == 0);

		return reason;
	}
};

/*
BNVM bytecode file format (Version 1.0.0):

 BNVM - 4 bytes
 version - 4 bytes
 globalVar size - 4 bytes
 globalVar count - 4 bytes
 for each globalVar:
   4 bytes - var name offset
   4 bytes - var name length (including null)
   N bytes - var name (includes null)

 funcDef count
 for each funcDef:
   4 bytes - code pointer
   4 bytes - def name length (including null)
   N bytes - def name (including null)


 externFunc count
   4 bytes - func index
   4 bytes - func name length (including null)
   N bytes - func name (including null)

   4 bytes - code size
   N bytes - code values

  ~BNVM - 4 bytes

*/

#define BNVM_VERSION 100

// TODO: Limits us to only 256 source files in one BNVM's code
#define BNVM_PACK_FILE_LINE(file, line) ((((file) & 0xFF) << 24) | ((line) & 0xFFFFFF))
#define BNVM_UNPACK_FILE(pair) ((pair) >> 24)
#define BNVM_UNPACK_LINE(pair) ((pair) & 0xFFFFFF)

// You know, sometimes C++ is alright.
static_assert(BNVM_UNPACK_FILE(BNVM_PACK_FILE_LINE(12,   68)) == 12, "Unpack file works");
static_assert(BNVM_UNPACK_FILE(BNVM_PACK_FILE_LINE(0, 2)) == 0, "Unpack file works");
static_assert(BNVM_UNPACK_FILE(BNVM_PACK_FILE_LINE(0,    68)) == 0, "Unpack file works");
static_assert(BNVM_UNPACK_LINE(BNVM_PACK_FILE_LINE(255, 228)) == 228, "Unpack line works");
static_assert(BNVM_UNPACK_LINE(BNVM_PACK_FILE_LINE(12,   68)) == 68, "Unpack line works");

struct StringDataHelper {
	SubString substr;
	int offset;
};

struct BNVM {
	Vector<byte> code;

	StringMap<int> functionPointers;

	Vector<StringDataHelper> stringOffsets;

	BNVMInstance inst;

	Vector<ExternFunc*> externFunctions;
	StringMap<int> externFuncIndices;

	StringMap<int> debugFileIndices;

	BNVM() {
		inst.vm = this;
	}

	void InitNewInst(BNVMInstance* newInst);

	void RegisterExternFunc(const char* name, ExternFunc* func);

	BNVMReturnReason Execute(const char* funcName);
	BNVMReturnReason ExecuteInternal();
	BNVMReturnReason ExecuteInternalFunc(const char* funcName) {
		return inst.ExecuteInternalFunc(funcName);
	}

	void WriteByteCodeToMemStream(MemStream* stream);
	void WriteByteCodeToFile(const char* fileName);

	void ReadByteCodeFromMemStream(MemStream* stream);
	void ReadByteCodeFromFile(const char* fileName);

	template<typename T>
	T GetGlobalVariableValue(const char* name) {
		return inst.GetGlobalVariableValue<T>(name);
	}

	template<typename T>
	T GetGlobalVariableValueByOffset(int offset) {
		return inst.GetGlobalVariableValueByOffset<T>(offset);
	}

	template<typename Arg, typename Ret>
	Ret ExecuteTyped(const char* funcName, const Arg& arg) {
		return inst.ExecuteTyped<Arg, Ret>(funcName, arg);
	}

	template<typename Arg>
	BNVMReturnReason ExecuteTyped(const char* funcName, const Arg& arg) {
		return inst.ExecuteTyped<Arg>(funcName, arg);
	}
};


#endif
