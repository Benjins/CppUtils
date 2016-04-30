#include "bnvparser.h"
#include "bnvm.h"
#include "lexer.h"
#include "assert.h"
#include "macros.h"

struct BuiltinOp{
	const char* name;
	Instruction intInstr;
	Instruction fltInstr;
};

BuiltinOp builtinOps[] = {
	{"*", I_MULTI, I_MULTF},
	{"+", I_ADDI,  I_ADDF},
	{"-", I_SUBI,  I_SUBF},
	{"/", I_DIVI,  I_DIVF },
	{"==", I_EQI,  I_EQF },
	{"!=", I_NEQI, I_NEQF },
	{"<", I_LTI, I_LTF },
	{"<=", I_LTEI, I_LTEF },
	{">", I_GTI, I_GTF },
	{">=", I_GTEI, I_GTEF}
};

struct BuiltinFunc{
	const char* name;
	const char* retType;
	const char* argTypes[3];
	Instruction instr;
};

BuiltinFunc builtinFuncs[] = {
	{ "PRINTI", "void",{ "int" }, I_PRINTI },
	{ "PRINTF", "void",{ "float" }, I_PRINTF },
	{ "READI", "int", {}, I_READI },
	{ "READF", "float", {}, I_READF }
};

bool FindBuiltinFunc(const SubString& name, BuiltinFunc* outFunc) {
	for (int i = 0; i < BNS_ARRAY_COUNT(builtinFuncs); i++) {
		if (name == builtinFuncs[i].name) {
			*outFunc = builtinFuncs[i];
			return true;
		}
	}

	return false;
}

bool FindBuiltinOp(const SubString& name, BuiltinOp* outInstr){
	for(int i = 0; i < BNS_ARRAY_COUNT(builtinOps); i++){
		if(name == builtinOps[i].name){
			*outInstr = builtinOps[i];
			return true;
		}
	}

	return false;
}

struct BuiltinType {
	const char* name;
	int size;
};

BuiltinType builtinTypes[] = {
	{ "int", 4 },
	{ "float", 4 },
	{ "void", 0 }
};

enum OperatorArity {
	OA_UNARY,
	OA_BINARY
};

struct OperatorPrecedence {
	const char* name;
	OperatorArity arity;
	int prec;
};

OperatorPrecedence opPrec[] = {
	{ ".",  OA_BINARY, 1 },
	{ "!",  OA_UNARY, 3 },
	{ "*",  OA_BINARY, 5 },
	{ "/",  OA_BINARY, 5 },
	{ "%",  OA_BINARY, 5 },
	{ "+",  OA_BINARY, 6 },
	{ "-",  OA_BINARY, 6 },
	{ ">", OA_BINARY,  9 },
	{ ">=", OA_BINARY,  9 },
	{ "<", OA_BINARY,  9 },
	{ "<=", OA_BINARY,  9 },
	{ "==", OA_BINARY,  9 },
	{ "!=", OA_BINARY,  9 },
	{ "&&", OA_BINARY,  13 },
	{ "||", OA_BINARY,  14 }
};

StringMap<int> operatorPrecedence;
StringMap<OperatorArity> operatorArity;

int GetOperatorPrecedence(const SubString& opName) {
	if (operatorPrecedence.count == 0) {
		for (int i = 0; i < BNS_ARRAY_COUNT(opPrec); i++) {
			operatorPrecedence.Insert(opPrec[i].name, opPrec[i].prec);
			operatorArity.Insert(opPrec[i].name, opPrec[i].arity);
		}
	}

	int prec = 0;
	operatorPrecedence.LookUp(opName, &prec);

	return prec;
}

BNVParser::BNVParser(){
	cursor = 0;
	currScope = nullptr;

	for(int i = 0; i < BNS_ARRAY_COUNT(builtinTypes); i++){
		TypeInfo* builtinType = new TypeInfo();
		String typeNameStr = builtinTypes[i].name;
		builtinType->size = builtinTypes[i].size;
		builtinType->typeName = typeNameStr.GetSubString(0, typeNameStr.GetLength());
		definedTypes.Insert(builtinTypes[i].name, builtinType);
	}

	for (int i = 0; i < BNS_ARRAY_COUNT(builtinFuncs); i++) {
		FuncDef* builtinDef = new FuncDef();
		String nameStr = builtinFuncs[i].name;
		builtinDef->name = nameStr.GetSubString(0, nameStr.GetLength());
		definedTypes.LookUp(builtinFuncs[i].retType, &builtinDef->retType);

		for (int j = 0; j < BNS_ARRAY_COUNT(builtinFuncs[i].argTypes); j++) {
			const char* argType = builtinFuncs[i].argTypes[j];
			if (argType != nullptr && *argType != '\0') {
				VarDecl decl;
				decl.name = String("blahblagh").GetSubString(0, 3);
				definedTypes.LookUp(argType, &decl.type);
				builtinDef->params.PushBack(decl);
			}
			else{
				break;
			}
		}

		funcDefs.PushBack(builtinDef);
	}
}

void BNVParser::ParseFile(const char* fileName){
	String str = ReadStringFromFile(fileName);

	Vector<SubString> lexedTokens = LexString(str);
	String fileNamePersistent = fileName;

	toks.Clear();
	toks.EnsureCapacity(lexedTokens.count);
	for(int i = 0; i < lexedTokens.count; i++){
		BNVToken tok;
		tok.file = fileNamePersistent;
		tok.substr = lexedTokens.data[i];
		toks.PushBack(tok);
	}

	cursor = 0;
	while(cursor < toks.count){
		if(StructDef* structDef = ParseStructDef()){
			structDefs.PushBack(structDef);
		}
		else if(FuncDef* funcDef = ParseFuncDef()){
			//funcDefs.PushBack(funcDef);
			//Do nothing
		}
		else{
			printf("\nWelp.............\n");
			break;
			//ERROR
		}
	}
}

StructDef* BNVParser::ParseStructDef(){
	PushCursorFrame();

	StructDef* def = new StructDef();
	SubString defName;
	if(ExpectAndEatWord("struct") && ParseIdentifier(&defName) && ExpectAndEatWord("{")){
		def->name = defName;
		VarDecl fieldDef;
		while(ExpectAndEatVarDecl(&fieldDef)){
			if(ExpectAndEatWord(";")){
				def->fields.PushBack(fieldDef);
			}
			else{
				PopCursorFrame();
				return false;
			}
		}

		if(ExpectAndEatWord("}") && ExpectAndEatWord(";")){
			return def;
		}
		else{
			PopCursorFrame();
			return nullptr;
		}
	}
	else{
		PopCursorFrame();
		return nullptr;
	}
}

FuncDef* BNVParser::ParseFuncDef(){
	PushCursorFrame();
	PushVarFrame();
	
	FuncDef* def = new FuncDef();
	if (TypeInfo* retType = ParseType()){
		SubString funcName;
		if(ParseIdentifier(&funcName)){
			def->retType = retType;
			def->name = funcName;
			if(ExpectAndEatWord("(")){
				if(ExpectAndEatWord(")")){
					// Do nothing
				}
				else{
					VarDecl funcParam;
					while(ExpectAndEatVarDecl(&funcParam)){
						def->params.PushBack(funcParam);
						varsInScope.PushBack(funcParam);
						
						if(ExpectAndEatWord(",")){
							//Do nothing
						}
						else if(ExpectAndEatWord(")")){
							break;
						}
						else{
							//ERROR
							printf("\nWelp, something went wrong...\n");
							break;
						}
					}
				}

				funcDefs.PushBack(def);
				
				if(Scope* scope = ParseScope()){
					def->statements = scope->statements;
					PopVarFrame();
					return def;
				}
				else{
					funcDefs.PopBack();
					PopCursorFrame();
					PopVarFrame();
					return nullptr;
				}
			}
			else {
				PopCursorFrame();
				PopVarFrame();
				return nullptr;
			}
		}
		else{
			PopCursorFrame();
			PopVarFrame();
			return nullptr;
		}
	}
	else{
		PopCursorFrame();
		PopVarFrame();
		return nullptr;
	}
}

Statement* BNVParser::ParseStatement(){
	PushCursorFrame();
	VarDecl decl;
	if(ExpectAndEatVarDecl(&decl)){
		varsInScope.PushBack(decl);
		//Add variable name
		cursor--;
		
		if(Assignment* assignment = ParseAssignment()){
			return assignment;
		}
		else{
			varsInScope.PopBack();
			PopCursorFrame();
			return nullptr;
		}
	}
	else if(ReturnStatement* retStmt = ParseReturnStatement()){
		return retStmt;
	}
	else if(IfStatement* ifStmt = ParseIfStatement()){
		return ifStmt;
	}
	else if(WhileStatement* whileStmt = ParseWhileStatement()){
		return whileStmt;
	}
	else if(Scope* scope = ParseScope()){
		return scope;
	}
	else if (Assignment* assign = ParseAssignment()) {
		return assign;
	}
	else if (Value* val = ParseValue()) {
		if (ExpectAndEatWord(";")) {
			return val;
		}
		else {
			delete val;
			PopCursorFrame();
			return nullptr;
		}
	}
	else{
		PopCursorFrame();
		return nullptr;
	}
}

Scope* BNVParser::ParseScope(){
	PushCursorFrame();
	PushVarFrame();
	
	Scope* scope = new Scope();
	
	if(ExpectAndEatWord("{")){
		while(Statement* stmt = ParseStatement()){
			scope->statements.PushBack(stmt);
		}
		
		if(ExpectAndEatWord("}")){
			for (int i = 0; i < scope->statements.count; i++) {
				if (scope->statements.data[i]->TypeCheck(*this) == (TypeInfo*)0x01) {
					PopVarFrame();
					PopCursorFrame();
					return nullptr;
				}
			}

			PopVarFrame();
			return scope;
		}
		else{
			PopVarFrame();
			PopCursorFrame();
			return nullptr;
		}
	}
	else{
		PopVarFrame();
		PopCursorFrame();
		return nullptr;
	}
}

ReturnStatement* BNVParser::ParseReturnStatement(){
	PushCursorFrame();
	
	if(ExpectAndEatWord("return")){
		if (Value* val = ParseValue()) {
			if (ExpectAndEatWord(";")) {
				ReturnStatement* ret = new ReturnStatement();
				ret->retVal = val;
				return ret;
			}
			else {
				PopCursorFrame();
				return nullptr;
			}
		}
		else {
			PopCursorFrame();
			return nullptr;
		}
	}
	else{
		PopCursorFrame();
		return nullptr;
	}
}

IfStatement* BNVParser::ParseIfStatement(){
	PushCursorFrame();
	if (ExpectAndEatWord("if")) {
		if (Value* val = ParseValue()) {
			IfStatement* ifStmt = new IfStatement();
			ifStmt->check = val;
			if (Scope* scope = ParseScope()) {
				ifStmt->statements = scope->statements;
				return ifStmt;
			}
			else {
				delete ifStmt;
				PopCursorFrame();
				return nullptr;
			}
		}
		else {
			PopCursorFrame();
			return nullptr;
		}
	}
	else {
		PopCursorFrame();
		return nullptr;
	}
}

WhileStatement* BNVParser::ParseWhileStatement(){
	PushCursorFrame();
	if (ExpectAndEatWord("while")) {
		if (Value* val = ParseValue()) {
			WhileStatement* whileStmt = new WhileStatement();
			whileStmt->check = val;
			if (Scope* scope = ParseScope()) {
				whileStmt->statements = scope->statements;
				return whileStmt;
			}
			else {
				delete whileStmt;
				PopCursorFrame();
				return nullptr;
			}
		}
		else {
			PopCursorFrame();
			return nullptr;
		}
	}
	else {
		PopCursorFrame();
		return nullptr;
	}
}

Assignment* BNVParser::ParseAssignment(){
	PushCursorFrame();

	if (TypeInfo* varType = ParseVarName()) {
		SubString varName = toks.data[cursor - 1].substr;
		if (ExpectAndEatWord("=")) {
			if (Value* val = ParseValue()) {
				if (ExpectAndEatWord(";")) {
					Assignment* assgn = new Assignment();
					assgn->val = val;
					assgn->varName = varName;
					return assgn;
				}
				else {
					delete varType;
					PopCursorFrame();
					return nullptr;
				}
			}
			else {
				delete varType;
				PopCursorFrame();
				return nullptr;
			}
		}
		else {
			delete varType;
			PopCursorFrame();
			return nullptr;
		}
	}
	else {
		PopCursorFrame();
		return nullptr;
	}
}

FuncDef* BNVParser::GetFuncDef(const SubString& name) const {
	for (int i = 0; i < funcDefs.count; i++) {
		if (funcDefs.data[i]->name == name) {
			return funcDefs.data[i];
		}
	}

	return nullptr;
}

TypeInfo* BNVParser::GetVariableType(const SubString& name) const {
	for (int i = 0; i < varsInScope.count; i++) {
		if (varsInScope.data[i].name == name) {
			return varsInScope.data[i].type;
		}
	}

	return nullptr;
}

int BNVParser::GetVariableOffset(const SubString& name) const {
	int offset = 0;
	for (int i = 0; i < varsInScope.count; i++) {
		if (varsInScope.data[i].name == name) {
			return offset;
		}

		offset += varsInScope.data[i].type->size;
	}

	return -1;
}

int BNVParser::GetStackFrameOffset() const {
	int offset = 0;
	for (int i = 0; i < varsInScope.count; i++) {
		offset += varsInScope.data[i].type->size;
	}

	return offset;
}

bool BNVParser::ShuntingYard(const Vector<BNVToken>& inToks, Vector<BNVToken>& outToks) {
	Vector<BNVToken> opStack;

	outToks.EnsureCapacity(inToks.count);
	outToks.Clear();

	for (int i = 0; i < inToks.count; i++) {
		BNVToken tok = inToks.data[i];
		if(tok.substr.start[0] >= '0' && tok.substr.start[0] <= '9'){
			outToks.PushBack(tok);
		}
		else if (GetFuncDef(tok.substr) != nullptr) {
			opStack.PushBack(tok);
		}
		else if (tok.substr == ",") {
			while (opStack.count > 0) {
				BNVToken opTok = opStack.data[opStack.count - 1];
				opStack.PopBack();
				if (opTok.substr == "(") {
					opStack.PushBack(opTok);
					break;
				}
				else {
					if (opStack.count == 0) {
						outToks.Clear();
						return false;
					}

					outToks.PushBack(opTok);
				}
			}
		}
		else if (int opPrec = GetOperatorPrecedence(tok.substr)) {
			while (opStack.count > 0) {
				int otherPrec = GetOperatorPrecedence(opStack.data[opStack.count - 1].substr);

				if (otherPrec > 0 && otherPrec <= opPrec) {
					outToks.PushBack(opStack.data[opStack.count - 1]);
					opStack.PopBack();
				}
				else {
					break;
				}
			}

			opStack.PushBack(tok);
		}
		else if (tok.substr == "(") {
			opStack.PushBack(tok);
		}
		else if (tok.substr == ")") {
			bool foundMatchingParen = false;
			while (opStack.count > 0) {
				BNVToken opTok = opStack.data[opStack.count - 1];
				opStack.PopBack();

				if (opTok.substr == "(") {
					foundMatchingParen = true;
					if (opStack.count > 0 && GetFuncDef(opStack.data[opStack.count - 1].substr) != nullptr) {
						outToks.PushBack(opStack.data[opStack.count - 1]);
						opStack.PopBack();
					}
					break;
				}
				else {
					outToks.PushBack(opTok);
				}
			}

			if (!foundMatchingParen) {
				outToks.Clear();
				return false;
			}
		}
		else {
			outToks.PushBack(tok);
		}
	}

	while (opStack.count > 0) {
		BNVToken tok = opStack.data[opStack.count - 1];
		opStack.PopBack();
		if (tok.substr == "(" || tok.substr == ")") {
			outToks.Clear();
			return false;
		}
		else {
			outToks.PushBack(tok);
		}
	}

	return true;
}

TypeInfo* IntLiteral::TypeCheck(const BNVParser& parser) {
	parser.definedTypes.LookUp("int", &type);
	return type;
}

TypeInfo* FloatLiteral::TypeCheck(const BNVParser& parser) {
	parser.definedTypes.LookUp("float", &type);
	return type;
}

void FloatLiteral::AddByteCode(BNVM& vm) {
	vm.code.PushBack(I_FLTLIT);
	int litCursor = vm.code.count;
	for (int i = 0; i < sizeof(float); i++) {
		vm.code.PushBack(0);
	}

	*(float*)&vm.code.data[litCursor] = value;
}

Value* BNVParser::ParseValue(){
	PushCursorFrame();
	
	int valEnd = cursor;
	for (int i = cursor; i < toks.count; i++) {
		//HACK: I'm so sorry.
		if (toks.data[i].substr == ";" || toks.data[i].substr == "{") {
			valEnd = i;
			break;
		}
	}

	if (valEnd == cursor) {
		PopCursorFrame();
		return nullptr;
	}

	Vector<BNVToken> valToks;
	valToks.EnsureCapacity(valEnd - cursor);
	for (int i = cursor; i < valEnd; i++) {
		valToks.PushBack(toks.data[i]);
	}

	Vector<BNVToken> shuntedToks;
	if (ShuntingYard(valToks, shuntedToks)) {
		Vector<Value*> vals;
		for (int i = 0; i < shuntedToks.count; i++) {
			SubString tokStr = shuntedToks.data[i].substr;

			if (FuncDef* funcDef = GetFuncDef(tokStr)) {
				if (vals.count < funcDef->params.count) {
					PopCursorFrame();
					return nullptr;
				}
				else {
					FunctionCall* funcCall = new FunctionCall();
					funcCall->func = funcDef;
					for (int j = 0; j < funcDef->params.count; j++) {
						int index = vals.count - funcDef->params.count + j;
						funcCall->args.PushBack(vals.data[index]);
					}

					for (int j = 0; j < funcDef->params.count; j++) {
						vals.PopBack();
					}

					vals.PushBack(funcCall);
				}
			}
			else if (TypeInfo* varType = GetVariableType(tokStr)) {
				VariableAccess* varAccess = new VariableAccess();
				varAccess->info = varType;
				varAccess->varName = tokStr;

				vals.PushBack(varAccess);
			}
			else if (tokStr.start[0] >= '0' && tokStr.start[0] <= '9') {
				if (FindChar(tokStr.start, '.') != -1) {
					float val = atof(tokStr.start);
					FloatLiteral* fltLit = new FloatLiteral();
					fltLit->value = val;
					vals.PushBack(fltLit);
				}
				else {
					int val = Atoi(tokStr.start);
					IntLiteral* intLit = new IntLiteral();
					intLit->value = val;
					vals.PushBack(intLit);
				}
			}
			else {
				OperatorArity arity;
				if (operatorArity.LookUp(tokStr, &arity)) {
					if (arity == OA_UNARY) {
						if (vals.count >= 1) {
							UnaryOp* unOp = new UnaryOp();
							unOp->op = tokStr;
							unOp->val = vals.Back();
							vals.PopBack();
							vals.PushBack(unOp);
						}
						else {
							printf("Unary operator without enough operands.\n");
							PopCursorFrame();
							return nullptr;
						}
					}
					else if (arity == OA_BINARY) {
						if (vals.count >= 2) {
							BinaryOp* bOp = new BinaryOp();
							bOp->op = tokStr;
							bOp->rVal = vals.Back();
							vals.PopBack();
							bOp->lVal = vals.Back();
							vals.PopBack();
							vals.PushBack(bOp);
						}
						else {
							printf("Binary operator '%.*s' without enough operands.\n", tokStr.length, tokStr.start);
							PopCursorFrame();
							return nullptr;
						}
					}
					else {
						printf("Unknown operator arity '%.*s'.\n", tokStr.length, tokStr.start);
						PopCursorFrame();
						return nullptr;
					}
				}
				else {
					PopCursorFrame();
					return nullptr;
				}
			}
		}

		if (vals.count == 1) {
			cursor += valToks.count;
			return vals.Back();
		}
		else {
			printf("Parsing value with wrong number of values: %d.\n", vals.count);
			PopCursorFrame();
			return nullptr;
		}
	}
	else {
		PopCursorFrame();
		return nullptr;
	}

}

TypeInfo* BNVParser::ParseType(){
	TypeInfo* type = nullptr;

	if (definedTypes.LookUp(toks.data[cursor].substr, &type)) {
		cursor++;
		return type;
	}

	return nullptr;
}

TypeInfo* BNVParser::ParseVarName(){
	for (int i = 0; i < varsInScope.count; i++) {
		if (varsInScope.data[i].name == toks.data[cursor].substr) {
			cursor++;
			return varsInScope.data[i].type;
		}
	}

	return nullptr;
}

bool BNVParser::ParseIdentifier(SubString* outStr){
	const char** ops = GetOperators();
	for (int i = 0; i < GetOperatorCount(); i++) {
		if(toks.data[cursor].substr == ops[i]){
			return false;
		}
	}

	*outStr = toks.data[cursor].substr;
	cursor++;
	return true;
}

void ReturnStatement::AddByteCode(BNVM& vm) {
	retVal->AddByteCode(vm);
	vm.code.PushBack(I_RETURN);
}

TypeInfo* ReturnStatement::TypeCheck(const BNVParser& parser) {
	TypeInfo* retType = retVal->TypeCheck(parser);
	return retType;
}

void Scope::AddByteCode(BNVM& vm) {
	for (int i = 0; i < statements.count; i++) {
		statements.data[i]->AddByteCode(vm);
	}
}

void IfStatement::AddByteCode(BNVM& vm) {
	check->AddByteCode(vm);
	IntLiteral intLit;
	intLit.value = 0;
	intLit.AddByteCode(vm);
	int literalIndex = vm.code.count - 4;

	vm.code.PushBack(I_BRANCHIFZERO);
	Scope::AddByteCode(vm);

	int branchTo = vm.code.count;
	vm.code.data[literalIndex + 3] = branchTo % 256;
	vm.code.data[literalIndex + 2] = (branchTo / 256) % 256;
	vm.code.data[literalIndex + 1] = (branchTo / 65536) % 256;
}

void WhileStatement::AddByteCode(BNVM& vm) {
	int loopTo = vm.code.count;
	
	check->AddByteCode(vm);
	IntLiteral intLit;
	intLit.value = 0;
	intLit.AddByteCode(vm);
	int literalIndex = vm.code.count - 4;

	vm.code.PushBack(I_BRANCHIFZERO);
	Scope::AddByteCode(vm);

	IntLiteral lit;
	lit.value = loopTo;
	lit.AddByteCode(vm);

	vm.code.PushBack(I_BRANCH);

	int branchTo = vm.code.count;
	vm.code.data[literalIndex + 3] = branchTo % 256;
	vm.code.data[literalIndex + 2] = (branchTo / 256) % 256;
	vm.code.data[literalIndex + 1] = (branchTo / 65536) % 256;
}

void IntLiteral::AddByteCode(BNVM& vm) {
	vm.code.PushBack(I_INTLIT);
	vm.code.PushBack(value >> 24);
	vm.code.PushBack((value >> 16) % 256);
	vm.code.PushBack((value >> 8) % 256);
	vm.code.PushBack(value % 256);
}

void BinaryOp::AddByteCode(BNVM& vm) {
	rVal->AddByteCode(vm);
	lVal->AddByteCode(vm);

	BuiltinOp opInfo;
	FindBuiltinOp(op, &opInfo);
	if (type->typeName == "int") {
		vm.code.PushBack(opInfo.intInstr);
	}
	else{
		ASSERT(type->typeName == "float");
		vm.code.PushBack(opInfo.fltInstr);
	}
	
}

TypeInfo* BinaryOp::TypeCheck(const BNVParser& parser) {
	TypeInfo* rInfo = rVal->TypeCheck(parser);
	TypeInfo* lInfo = lVal->TypeCheck(parser);
	if (rInfo == lInfo && (rInfo->typeName == "int" || rInfo->typeName == "float")) {
		type = rInfo;
		return type;
	}
	else {
		return (TypeInfo*)0x01;
	}
}

void UnaryOp::AddByteCode(BNVM& vm) {
	
}

TypeInfo* UnaryOp::TypeCheck(const BNVParser& parser) {
	type = val->TypeCheck(parser);
	return type;
}

void Assignment::AddByteCode(BNVM& vm){
	val->AddByteCode(vm);
	IntLiteral lit;
	lit.value = regIndex;
	lit.AddByteCode(vm);

	if (val->type->typeName == "int") {
		vm.code.PushBack(I_STOREI);
	}
	else if (val->type->typeName == "float") {
		vm.code.PushBack(I_STOREF);
	}
	else {
		//TODO: struct fields
	}
}

TypeInfo* Assignment::TypeCheck(const BNVParser& parser) {
	if (regIndex == -1) {
		if (TypeInfo* varType = parser.GetVariableType(varName)) {
			TypeInfo* valType = val->TypeCheck(parser);

			if (valType != (TypeInfo*)0x01 && valType == varType) {
				regIndex = parser.GetVariableOffset(varName);
				return varType;
			}
		}

		return (TypeInfo*)0x01;
	}
	else {
		return val->type;
	}
}

void FunctionCall::AddByteCode(BNVM& vm) {
	for (int i = args.count - 1; i >= 0; i--) {
		args.data[i]->AddByteCode(vm);
	}

	BuiltinFunc builtinFunc;
	if (FindBuiltinFunc(func->name, &builtinFunc)) {
		vm.code.PushBack(builtinFunc.instr);
	}
	else {
		int funcPtr = -1;
		vm.functionPointers.LookUp(func->name, &funcPtr);
		IntLiteral lit;
		lit.value = funcPtr;
		lit.AddByteCode(vm);

		lit.value = stackFrameOffset;
		lit.AddByteCode(vm);

		vm.code.PushBack(I_CALL);
	}
}

TypeInfo* FunctionCall::TypeCheck(const BNVParser& parser) {
	if (func->params.count != args.count){
		return (TypeInfo*)0x01;
	}

	for (int i = 0; i < args.count; i++) {
		TypeInfo* argInfo = args.data[i]->TypeCheck(parser);
		if (argInfo == (TypeInfo*)0x01 || argInfo != func->params.data[i].type) {
			return (TypeInfo*)0x01;
		}
	}

	stackFrameOffset = parser.GetStackFrameOffset();
	type = func->retType;
	return type;
}

void VariableAccess::AddByteCode(BNVM& vm) {
	IntLiteral lit;
	lit.value = regOffset;
	lit.AddByteCode(vm);

	if (type->typeName == "int") {
		vm.code.PushBack(I_LOADI);
	}
	else if (type->typeName == "float") {
		vm.code.PushBack(I_LOADF);
	}
	else {
		// TODO: struct accesses
	}
}

TypeInfo* VariableAccess::TypeCheck(const BNVParser& parser) {
	if (regOffset == -1) {
		if (TypeInfo* varType = parser.GetVariableType(varName)) {
			type = varType;
			regOffset = parser.GetVariableOffset(varName);
			return type;
		}
		else {
			return (TypeInfo*)0x01;
		}
	}
	else {
		return type;
	}
}

void FuncDef::AddByteCode(BNVM& vm) {
	vm.functionPointers.Insert(name, vm.code.count);

	for (int i = 0; i < params.count; i++) {
		IntLiteral lit;
		lit.value = i;
		lit.AddByteCode(vm);

		//TODO: types;
		vm.code.PushBack(I_STOREI);
	}

	Scope::AddByteCode(vm);
}

TypeInfo* FuncDef::TypeCheck(const BNVParser& parser) {
	return Scope::TypeCheck(parser);
}

TypeInfo* Scope::TypeCheck(const BNVParser& parser) {
	for (int i = 0; i < statements.count; i++) {
		if (statements.data[i]->TypeCheck(parser) == (TypeInfo*)0x01) {
			return (TypeInfo*)0x01;
		}
	}

	return nullptr;
}

TypeInfo* IfStatement::TypeCheck(const BNVParser& parser) {
	TypeInfo* intInfo;
	parser.definedTypes.LookUp("int", &intInfo);
	if (check->TypeCheck(parser) != intInfo) {
		return (TypeInfo*)0x01;
	}

	return Scope::TypeCheck(parser);
}

void BNVParser::AddByteCode(BNVM& vm) {
	for (int i = 0; i < funcDefs.count; i++) {
		BuiltinFunc builtinFunc;
		if (!FindBuiltinFunc(funcDefs.data[i]->name, &builtinFunc)) {
			funcDefs.data[i]->AddByteCode(vm);
		}
	}
}

bool BNVParser::TypeCheck() {
	TypeInfo* errorInfo = (TypeInfo*)0x01;
	for (int i = 0; i < funcDefs.count; i++) {
		TypeInfo* typeCheck = funcDefs.data[i]->TypeCheck(*this);
		if(typeCheck == errorInfo){
			return false;
		}
	}

	return true;
}

#if defined(BNVPARSER_TEST_MAIN)

int main(int argc, char** argv){
	BNS_UNUSED(argc);
	BNS_UNUSED(argv);

	BNVParser parser;

	parser.ParseFile("parserTest.bnv");

	ASSERT(parser.funcDefs.count == 7);
	ASSERT(parser.funcDefs.data[6]->name == "main");

	BNVM vm;
	parser.AddByteCode(vm);

	/*
	int facStart = 0;
	int facEnd = 0;
	vm.functionPointers.LookUp("main", &facEnd);
	for (int i = facStart; i < facEnd; i++) {
		printf("|%3d|\n", vm.code.data[i]);
	}
	*/

	vm.Execute("main");

	return 0;
}




#endif

