#include "bnvm.h"

#include "macros.h"

#include <stdio.h>

void MemStack::Increment(int amt){
	ASSERT(stackMem.count + amt < limit);
	stackMem.count += amt;
}

void MemStack::Decrement(int amt){
	ASSERT(stackMem.count >= amt);
	stackMem.count -= amt;
}

void MemStack::SetSize(int amt){
	ASSERT(0 <= amt && amt < limit);
	stackMem.count = amt;
}

int BNVM::Execute(const char* funcName){
	int startPc = -1;
	bool wasFound = functionPointers.LookUp(funcName, &startPc);
	
	ASSERT(wasFound && startPc >= 0 && startPc < code.count);
	
	pc = startPc;
	
	callStack.Push<int>(code.count);
	callStack.Push<int>(0);

	for(int i = startPc; i < code.count; i++){
		Instruction instr = (Instruction)code.data[i];
		switch(instr){
			
		case I_ADDI:{
			int a = tempStack.Pop<int>();
			int b = tempStack.Pop<int>();
			tempStack.Push(a+b);
		} break;
		
		case I_MULTI:{
			int a = tempStack.Pop<int>();
			int b = tempStack.Pop<int>();
			tempStack.Push(a*b);
		} break;
		
		case I_DIVI:{
			int a = tempStack.Pop<int>();
			int b = tempStack.Pop<int>();
			tempStack.Push(a/b);
		} break;
		
		case I_SUBI:{
			int a = tempStack.Pop<int>();
			int b = tempStack.Pop<int>();
			tempStack.Push(a-b);
		} break;
		
		case I_NEGATEI:{
			
		} break;
		
		case I_ADDF:{
			float a = tempStack.Pop<float>();
			float b = tempStack.Pop<float>();
			tempStack.Push(a + b);
		} break;
		
		case I_MULTF:{
			float a = tempStack.Pop<float>();
			float b = tempStack.Pop<float>();
			tempStack.Push(a * b);
		} break;
		
		case I_DIVF:{
			float a = tempStack.Pop<float>();
			float b = tempStack.Pop<float>();
			tempStack.Push(a / b);
		} break;
		
		case I_SUBF:{
			float a = tempStack.Pop<float>();
			float b = tempStack.Pop<float>();
			tempStack.Push(a - b);
		} break;
		
		case I_NEGATEF:{
			
		} break;
		
		case I_BRANCH:{
			int branchto = tempStack.Pop<int>();
			i = branchto - 1;
		} break;
		
		case I_BRANCHIFZERO:{
			int branchto = tempStack.Pop<int>();
			int check = tempStack.Pop<int>();
			if (!check) {
				i = branchto - 1;
			}
		} break;

		case I_BRANCHIFNOTZERO: {
			int branchto = tempStack.Pop<int>();
			int check = tempStack.Pop<int>();
			if (check) {
				i = branchto - 1;
			}
		} break;
		
		case I_CALL:{
			int stkFrameOffset = tempStack.Pop<int>();
			int jumpTo = tempStack.Pop<int>();

			callStack.Push<int>(i);
			callStack.Push<int>(varStack.stackMem.count);

			varStack.Increment(stkFrameOffset);
			i = jumpTo - 1;
		} break;
		
		case I_RETURN:{
			int oldStkFrame = callStack.Pop<int>();
			int returnAddr = callStack.Pop<int>();

			varStack.stackMem.count = oldStkFrame;
			i = returnAddr;
		} break;
		
		case I_PRINTI:{
			int val = tempStack.Pop<int>();
			printf("%d\n", val);
		} break;
		
		case I_READI:{
			int val = -1;
			scanf("%d", &val);
			tempStack.Push(val);
		} break;
		
		case I_PRINTF:{
			float val = tempStack.Pop<float>();
			printf("%f\n", val);
		} break;
		
		case I_READF:{
			float val = -1.2f;
			scanf("%f", &val);
			tempStack.Push(val);
		} break;
		
		case I_LOADI:{
			int reg = tempStack.Pop<int>();
			int val = *varStack.Access<int>(reg);
			tempStack.Push(val);
		} break;
		
		case I_STOREI:{
			int reg = tempStack.Pop<int>();
			int val = tempStack.Pop<int>();
			*varStack.Access<int>(reg) = val;
		} break;
		
		case I_INTLIT:{
			int a = code.data[i+1];
			int b = code.data[i+2];
			int c = code.data[i+3];
			int d = code.data[i+4];
			
			int val = (a << 24) | (b << 16) | (c << 8) | d;
			
			tempStack.Push(val);
			
			i += 4;
		} break;
		
		case I_FLTLIT:{
			float val = *(float*)&code.data[i + 1];
			tempStack.Push(val);
			i += 4;
		} break;
		
		case I_EQI: {
			tempStack.Push<int>(tempStack.Pop<int>() == tempStack.Pop<int>());
		} break;

		case I_EQF: {
			tempStack.Push<int>(tempStack.Pop<float>() == tempStack.Pop<float>());
		} break;
		
		case I_NEQI: {
			tempStack.Push<int>(tempStack.Pop<int>() != tempStack.Pop<int>());
		} break;

		case I_NEQF: {
			tempStack.Push<int>(tempStack.Pop<float>() != tempStack.Pop<float>());
		} break;

		case I_GTI: {
			tempStack.Push<int>(tempStack.Pop<int>() > tempStack.Pop<int>());
		} break;

		case I_GTF: {
			tempStack.Push<int>(tempStack.Pop<float>() > tempStack.Pop<float>());
		} break;

		case I_GTEI: {
			tempStack.Push<int>(tempStack.Pop<int>() >= tempStack.Pop<int>());
		} break;

		case I_GTEF: {
			tempStack.Push<int>(tempStack.Pop<float>() >= tempStack.Pop<float>());
		} break;

		case I_LTI: {
			tempStack.Push<int>(tempStack.Pop<int>() < tempStack.Pop<int>());
		} break;

		case I_LTF: {
			tempStack.Push<int>(tempStack.Pop<float>() < tempStack.Pop<float>());
		} break;

		case I_LTEI: {
			tempStack.Push<int>(tempStack.Pop<int>() <= tempStack.Pop<int>());
		} break;

		case I_LTEF: {
			tempStack.Push<int>(tempStack.Pop<float>() <= tempStack.Pop<float>());
		} break;

		}
	}
	
	if (tempStack.stackMem.count == 4) {
		return tempStack.Pop<int>();
	}
	else {
		return 0;
	}
}

#if defined(BNVM_TEST_MAIN)

int main(int argc, char** argv){
	BNS_UNUSED(argc);
	BNS_UNUSED(argv);
	
	BNVM vm;
	
	vm.functionPointers.Insert("main", 0);
	
	vm.code.PushBack(I_READI);
	vm.code.PushBack(I_INTLIT);
	vm.code.PushBack(0);
	vm.code.PushBack(0);
	vm.code.PushBack(0);
	vm.code.PushBack(5);
	vm.code.PushBack(I_MULTI);
	vm.code.PushBack(I_PRINTI);
	
	vm.Execute("main");
	
	return 0;
}

#endif

