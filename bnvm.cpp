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
	
	for(int i = startPc; i < code.count; i++){
		Instruction instr = (Instruction)code.data[i];
		switch(instr){
			
		case I_ADDI:{
			
		} break;
		
		case I_MULTI:{
			int a = tempStack.Pop<int>();
			int b = tempStack.Pop<int>();
			tempStack.Push(a*b);
		} break;
		
		case I_DIVI:{
			
		} break;
		
		case I_SUBI:{
			
		} break;
		
		case I_NEGATEI:{
			
		} break;
		
		case I_ADDF:{
			
		} break;
		
		case I_MULTF:{
			
		} break;
		
		case I_DIVF:{
			
		} break;
		
		case I_SUBF:{
			
		} break;
		
		case I_NEGATEF:{
			
		} break;
		
		case I_BRANCH:{
			
		} break;
		
		case I_BRANCHIFZERO:{
			
		} break;
		
		case I_CALL:{
			
		} break;
		
		case I_RETURN:{
			
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
			
		} break;
		
		case I_READF:{
			
		} break;
		
		case I_LOAD:{
			
		} break;
		
		case I_STORE:{
			
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
			
		} break;
		
		default:
			break;
		}
	}
	
	return -1;
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

