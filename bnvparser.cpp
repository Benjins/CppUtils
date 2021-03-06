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
	{ "PRINTS", "void",{ "string" }, I_PRINTS },
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
	{ "string", 4 },
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

int FindOffsetForString(const char* str, const Vector<StringDataHelper>& offsets) {
	for (int i = 0; i < offsets.count; i++) {
		// Compare by pointer value, since it's really just a key
		if (offsets.data[i].substr.start == str) {
			return offsets.data[i].offset;
		}
	}

	return -1;
}

BNVParser::BNVParser(){
	generateDebugInfo = false;
	cursor = 0;
	currentStringOffset = 0;

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

void AddDebugInfo(BNVM& vm, String& file, int line) {
	if (line > 0) {
		int fileIndex;
		if (!vm.debugFileIndices.LookUp(file, &fileIndex)) {
			fileIndex = vm.debugFileIndices.count;
			vm.debugFileIndices.Insert(file, fileIndex);
		}

		int newFileLine = BNVM_PACK_FILE_LINE(fileIndex, line);
		if (vm.inst.currentFileLine != newFileLine) {
			vm.code.PushBack(I_LINE);

			vm.code.PushBack(newFileLine % 256);
			vm.code.PushBack((newFileLine >> 8) % 256);
			vm.code.PushBack((newFileLine >> 16) % 256);
			vm.code.PushBack(newFileLine >> 24);

			vm.inst.currentFileLine = newFileLine;
		}
	}
}

Vector<BNVToken> BNVParser::ReadTokenizeProcessFile(String fileName) {
	String str = ReadStringFromFile(fileName.string);

	Vector<SubString> lexedTokens = LexString(str);

	Vector<BNVToken> processedToks;
	char* buffer = str.string;
	int lineNum = 1;
	for (int i = 0; i < lexedTokens.count; i++) {
		while (buffer < lexedTokens.data[i].start) {
			if (*buffer == '\n') {
				lineNum++;
			}
			
			buffer++;
		}

		if (lexedTokens.data[i] == "#") {
			if (lexedTokens.count > (i + 2) && lexedTokens.data[i+1] == "include") {
				SubString fileNameToken = lexedTokens.data[i + 2];

				if (fileNameToken.start[0] == '"') {
					StringStackBuffer<512> fullFileName("%s", fileName.string);
					int lastSlash = 0;
					for (int j = 0; j < fullFileName.length; j++) {
						if (fullFileName.buffer[j] == '/') {
							lastSlash = j;
						}
					}

					fullFileName.length = lastSlash;
					fullFileName.buffer[lastSlash] = '\0';
					if (fullFileName.length > 0) {
						fullFileName.Append("/");
					}

					//Append the include'd relative fileName, but trim the quotes
					fullFileName.AppendFormat("%.*s", fileNameToken.length - 2, fileNameToken.start + 1);

					Vector<BNVToken> includedToks = ReadTokenizeProcessFile(fullFileName.buffer);

					processedToks.InsertVector(processedToks.count, includedToks);
					i += 2;
				}
				else {
					ASSERT_WARN("Error when processing file '%s'", fileName.string);
					break;
				}
			}
			else {
				ASSERT_WARN("Error when processing file '%s'", fileName.string);
				break;
			}
		}
		else {
			BNVToken tok;
			tok.file = fileName;
			tok.line = lineNum;
			tok.substr = lexedTokens.data[i];
			processedToks.PushBack(tok);
		}
	}

	return processedToks;
}

void BNVParser::ParseFile(const char* fileName){
	toks = ReadTokenizeProcessFile(fileName);

	cursor = 0;
	while(cursor < toks.count){
		if(StructDef* structDef = ParseStructDef()){
			structDefs.PushBack(structDef);
			TypeInfo* typeInfo = new TypeInfo();
			typeInfo->typeName = structDef->name;
			typeInfo->size = structDef->GetSize();
			definedTypes.Insert(structDef->name, typeInfo);
		}
		else if(FuncDef* funcDef = ParseFuncDef()){
			BNS_UNUSED(funcDef);
			//funcDefs.PushBack(funcDef);
			//Do nothing
		}
		else{
			VarDecl globalDecl;
			if (ParseGlobalVar(&globalDecl)) {
				//Do nothing
			}
			else {
				printf("\nWelp.............\n");
				break;
				//ERROR
			}
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
				BNS_SAFE_DELETE(def);
				PopCursorFrame();
				return nullptr;
			}
		}

		if(ExpectAndEatWord("}") && ExpectAndEatWord(";")){
			return def;
		}
		else{
			BNS_SAFE_DELETE(def);
			PopCursorFrame();
			return nullptr;
		}
	}
	else{
		BNS_SAFE_DELETE(def);
		PopCursorFrame();
		return nullptr;
	}
}

FuncDef* BNVParser::ParseFuncDef(){
	PushCursorFrame();
	PushVarFrame();

	FuncDef* def = nullptr;
	bool isExtern = false;
	if (ExpectAndEatWord("extern")) {
		def = new ExternFuncDef();
		isExtern = true;
	}
	else {
		def = new FuncDef();
	}

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

				if (isExtern) {
					PopVarFrame();
					if (ExpectAndEatWord(";")) {
						externFuncNames.Insert(def->name, externFuncNames.count);
						return def;
					}
					else {
						funcDefs.PopBack();
						BNS_SAFE_DELETE(def);
						return nullptr;
					}
				}

				if(Scope* scope = ParseScope()){
					def->statements = scope->statements;
					if (def->retType->typeName == "void") {
						ReturnStatement* retStmt = new ReturnStatement();
						retStmt->retVal = new VoidLiteral();
						def->statements.PushBack(retStmt);
					}
					//Because I'm too lazy to write a move constructor
					scope->statements.count = 0;
					BNS_SAFE_DELETE(scope);
					PopVarFrame();

					return def;
				}
				else{
					funcDefs.PopBack();
					PopCursorFrame();
					PopVarFrame();
					BNS_SAFE_DELETE(def);
					return nullptr;
				}
			}
			else {
				PopCursorFrame();
				PopVarFrame();
				BNS_SAFE_DELETE(def);
				return nullptr;
			}
		}
		else{
			PopCursorFrame();
			PopVarFrame();
			BNS_SAFE_DELETE(def);
			return nullptr;
		}
	}
	else{
		PopCursorFrame();
		PopVarFrame();
		BNS_SAFE_DELETE(def);
		return nullptr;
	}
}

bool BNVParser::ParseGlobalVar(VarDecl* outDecl) {
	VarDecl tempDecl;
	if (ExpectAndEatVarDecl(&tempDecl)) {
		if (ExpectAndEatWord(";")) {
			globalVars.PushBack(tempDecl);
			*outDecl = tempDecl;
			return true;
		}
	}

	return false;
}

Statement* BNVParser::ParseStatement(){
	PushCursorFrame();
	VarDecl decl;

	int startingTokIdx = cursor;

#define RETURN_STMT(stmt) \
	if (generateDebugInfo) { \
		BNVToken tok = toks.data[startingTokIdx];\
		/*printf("Generating debug info for token '%.*s' on line %d\n", tok.substr.length, tok.substr.start, tok.line); */\
		stmt->file = tok.file; \
		stmt->line = tok.line; \
	} \
	return stmt

	if(ExpectAndEatVarDecl(&decl)){
		varsInScope.PushBack(decl);

		if (ExpectAndEatWord(";")) {
			VariableDeclaration* varDecl = new VariableDeclaration();
			RETURN_STMT(varDecl);
		}

		//Add variable name
		cursor--;
		
		if(Assignment* assignment = ParseAssignment()){
			RETURN_STMT(assignment);
		}
		else{
			varsInScope.PopBack();
			PopCursorFrame();
			return nullptr;
		}
	}
	else if(ReturnStatement* retStmt = ParseReturnStatement()){
		RETURN_STMT(retStmt);
	}
	else if(IfStatement* ifStmt = ParseIfStatement()){
		RETURN_STMT(ifStmt);
	}
	else if(WhileStatement* whileStmt = ParseWhileStatement()){
		RETURN_STMT(whileStmt);
	}
	else if(Scope* scope = ParseScope()){
		RETURN_STMT(scope);
	}
	else if (Assignment* assign = ParseAssignment()) {
		RETURN_STMT(assign);
	}
	else if (Value* val = ParseValue()) {
		if (ExpectAndEatWord(";")) {
			RETURN_STMT(val);
		}
		else {
			BNS_SAFE_DELETE(val);
			PopCursorFrame();
			return nullptr;
		}
	}
	else{
		PopCursorFrame();
		return nullptr;
	}

#undef RETURN_STMT
}

Scope* BNVParser::ParseScope(){
	PushCursorFrame();
	PushVarFrame();

	if(ExpectAndEatWord("{")){
		Scope* scope = new Scope();
		while(Statement* stmt = ParseStatement()){
			scope->statements.PushBack(stmt);
		}

		if(ExpectAndEatWord("}")){
			for (int i = 0; i < scope->statements.count; i++) {
				if (scope->statements.data[i]->TypeCheck(*this) == (TypeInfo*)0x01) {
					BNS_SAFE_DELETE(scope);
					PopVarFrame();
					PopCursorFrame();
					return nullptr;
				}
			}

			PopVarFrame();
			return scope;
		}
		else{
			BNS_SAFE_DELETE(scope);
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
				scope->statements.count = 0;
				BNS_SAFE_DELETE(scope);
				return ifStmt;
			}
			else {
				BNS_SAFE_DELETE(ifStmt);
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
				scope->statements.count = 0;
				BNS_SAFE_DELETE(scope);
				return whileStmt;
			}
			else {
				BNS_SAFE_DELETE(whileStmt);
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

		VariableAccess* varAccess = new VariableAccess();
		varAccess->varName = varName;
		varAccess->isGlobal = VarIsGlobal(varName);
		varAccess->type = varType;

		StructDef* varStruct = GetStructDef(varType->typeName);
		while (varStruct && ExpectAndEatWord(".")) {
			for (int i = 0; i < varStruct->fields.count; i++) {
				if (varStruct->fields.data[i].name == toks.data[cursor].substr) {
					TypeInfo* fieldType = varStruct->fields.data[i].type;

					FieldAccess* fieldAccess = new FieldAccess();
					fieldAccess->var = varAccess;
					fieldAccess->varName = varAccess->varName;
					fieldAccess->isGlobal = varAccess->isGlobal;
					fieldAccess->fieldName = varStruct->fields.data[i].name;
					fieldAccess->type = fieldType;
					varAccess = fieldAccess;
					
					varStruct = GetStructDef(fieldType->typeName);
					cursor++;

					break;
				}
			}
		}

		if (ExpectAndEatWord("=")) {
			if (Value* val = ParseValue()) {
				if (ExpectAndEatWord(";")) {
					Assignment* assgn = new Assignment();
					assgn->val = val;
					assgn->var = varAccess;
					return assgn;
				}
				else {
					BNS_SAFE_DELETE(varType);
					PopCursorFrame();
					return nullptr;
				}
			}
			else {
				BNS_SAFE_DELETE(varType);
				PopCursorFrame();
				return nullptr;
			}
		}
		else {
			BNS_SAFE_DELETE(varType);
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

StructDef* BNVParser::GetStructDef(const SubString& name) const {
	for (int i = 0; i < structDefs.count; i++) {
		if (structDefs.data[i]->name == name) {
			return structDefs.data[i];
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

	for (int i = 0; i < globalVars.count; i++) {
		if (globalVars.data[i].name == name) {
			return globalVars.data[i].type;
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

int BNVParser::GetGlobalVariableOffset(const SubString& name) const {
	int offset = 0;
	for (int i = 0; i < globalVars.count; i++) {
		if (globalVars.data[i].name == name) {
			return offset;
		}

		offset += globalVars.data[i].type->size;
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

int BNVParser::GetGlobalVarSize() const {
	int offset = 0;
	for (int i = 0; i < globalVars.count; i++) {
		offset += globalVars.data[i].type->size;
	}

	return offset;
}

bool BNVParser::VarIsGlobal(const SubString& name) const {
	for (int i = 0; i < globalVars.count; i++) {
		if (globalVars.data[i].name == name) {
			return true;
		}
	}

	return false;
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

TypeInfo* IntLiteral::TypeCheck(BNVParser& parser) {
	parser.definedTypes.LookUp("int", &type);
	return type;
}

TypeInfo* FloatLiteral::TypeCheck(BNVParser& parser) {
	parser.definedTypes.LookUp("float", &type);
	return type;
}

TypeInfo* StringLiteral::TypeCheck(BNVParser& parser) {
	parser.definedTypes.LookUp("string", &type);

	StringDataHelper helper;
	helper.substr = value;
	helper.offset = parser.currentStringOffset;
	parser.stringOffsets.PushBack(helper);
	parser.currentStringOffset += (value.length + 1);

	return type;
}

void FloatLiteral::AddByteCode(BNVM& vm) {
	AddDebugInfo(vm, file, line);
	// Ensure that floats are aligned to 4 bytes.
	int codeSize = vm.code.count;
	int noopCount = 3 - (codeSize % 4);
	for (int i = 0; i < noopCount; i++){
		vm.code.PushBack(I_NOOP);
	}

	vm.code.PushBack(I_FLTLIT);

	int litCursor = vm.code.count;
	for (unsigned int i = 0; i < sizeof(float); i++) {
		vm.code.PushBack(0);
	}

	*(float*)&vm.code.data[litCursor] = value;
}

TypeInfo* VoidLiteral::TypeCheck(BNVParser& parser){
	TypeInfo* info;
	parser.definedTypes.LookUp("void", &info);
	return info;
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
					printf("Calling '%.*s' with wrong number of params.\n", funcDef->name.length, funcDef->name.start);
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
				BNS_UNUSED(varType);
				VariableAccess* varAccess = new VariableAccess();
				varAccess->isGlobal = VarIsGlobal(tokStr);
				varAccess->varName = tokStr;

				vals.PushBack(varAccess);
			}
			else if (tokStr == ".") {
				Value* val = vals.Back();
				vals.PopBack();
				VariableAccess* varAccess = dynamic_cast<VariableAccess*>(val);

				if (varAccess == nullptr) {
					printf("Found '.' on non-variable value.\n");
					return nullptr;
				}
				else {
					FieldAccess* fieldAccess = new FieldAccess();
					i++;
					fieldAccess->fieldName = shuntedToks.data[i].substr;
					fieldAccess->var = varAccess;
					fieldAccess->varName = varAccess->varName;
					fieldAccess->isGlobal = varAccess->isGlobal;

					vals.PushBack(fieldAccess);
				}
			}
			else if (tokStr.start[0] >= '0' && tokStr.start[0] <= '9') {
				int idx = FindChar(tokStr.start, '.');
				if (idx != -1 && idx < tokStr.length) {
					float val = Atof(tokStr.start);
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
			else if (tokStr.start[0] == '"') {
				StringLiteral* strLit = new StringLiteral();
				strLit->value = tokStr;
				strLit->value.start++;
				strLit->value.length -= 2;
				vals.PushBack(strLit);
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
					for(int j = 0; j < vals.count; j++){
						BNS_SAFE_DELETE(vals.data[j]);
					}
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

	for (int i = 0; i < globalVars.count; i++) {
		if (globalVars.data[i].name == toks.data[cursor].substr) {
			cursor++;
			return globalVars.data[i].type;
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
	AddDebugInfo(vm, file, line);
	retVal->AddByteCode(vm);
	vm.code.PushBack(I_RETURN);
}

ReturnStatement::~ReturnStatement(){
	BNS_SAFE_DELETE(retVal);
}

TypeInfo* ReturnStatement::TypeCheck(BNVParser& parser) {
	TypeInfo* retType = retVal->TypeCheck(parser);
	return retType;
}

void Scope::AddByteCode(BNVM& vm) {
	for (int i = 0; i < statements.count; i++) {
		statements.data[i]->AddByteCode(vm);
	}
}

void IfStatement::AddByteCode(BNVM& vm) {
	AddDebugInfo(vm, file, line);
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

IfStatement::~IfStatement(){
	BNS_SAFE_DELETE(check);
}

void WhileStatement::AddByteCode(BNVM& vm) {
	int loopTo = vm.code.count;
	
	AddDebugInfo(vm, file, line);
	
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
	AddDebugInfo(vm, file, line);
	vm.code.PushBack(I_INTLIT);
	vm.code.PushBack(value >> 24);
	vm.code.PushBack((value >> 16) % 256);
	vm.code.PushBack((value >> 8) % 256);
	vm.code.PushBack(value % 256);
}

void StringLiteral::AddByteCode(BNVM& vm) {
	AddDebugInfo(vm, file, line);
	vm.code.PushBack(I_STRINGLIT);

	int offset = FindOffsetForString(value.start, vm.stringOffsets);
	vm.code.PushBack(offset >> 24);
	vm.code.PushBack((offset >> 16) % 256);
	vm.code.PushBack((offset >> 8) % 256);
	vm.code.PushBack(offset % 256);
}

void BinaryOp::AddByteCode(BNVM& vm) {
	AddDebugInfo(vm, file, line);
	rVal->AddByteCode(vm);
	if (rVal->type->typeName == "int" && lVal->type->typeName == "float") {
		vm.code.PushBack(I_ITOF);
	}
	lVal->AddByteCode(vm);
	if (rVal->type->typeName == "float" && lVal->type->typeName == "int") {
		vm.code.PushBack(I_ITOF);
	}

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

TypeInfo* BinaryOp::TypeCheck(BNVParser& parser) {
	TypeInfo* rInfo = rVal->TypeCheck(parser);
	TypeInfo* lInfo = lVal->TypeCheck(parser);

	if (rInfo == (TypeInfo*)0x01 || lInfo == (TypeInfo*)0x01) {
		return (TypeInfo*)0x01;
	}

	if ((lInfo->typeName == "int" || lInfo->typeName == "float") 
	 && (rInfo->typeName == "int" || rInfo->typeName == "float")) {
		if (op == "<" || op == "==" || op == "<=" || op == ">" || op == ">=") {
			parser.definedTypes.LookUp("int", &type);
		}
		else {
			type = rInfo;
		}
		return type;
	}
	else {
		return (TypeInfo*)0x01;
	}
}

void UnaryOp::AddByteCode(BNVM& vm) {
	AddDebugInfo(vm, file, line);
	val->AddByteCode(vm);
	ASSERT_WARN("%s is not yet implemented.\n", __FUNCTION__);
}

TypeInfo* UnaryOp::TypeCheck(BNVParser& parser) {
	type = val->TypeCheck(parser);
	return type;
}

void Assignment::AddByteCode(BNVM& vm){
	AddDebugInfo(vm, file, line);
	val->AddByteCode(vm);
	for (int i = 0; i < var->type->size; i += 4) {
		IntLiteral lit;
		lit.value = var->regOffset + i;
		lit.AddByteCode(vm);

		if (var->isGlobal) {
			vm.code.PushBack(I_STOREG);
		}
		else {
			vm.code.PushBack(I_STOREI);
		}
	}
}

TypeInfo* Assignment::TypeCheck(BNVParser& parser) {
	TypeInfo* varInfo = val->TypeCheck(parser);
	if (varInfo == (TypeInfo*)0x01) {
		return (TypeInfo*)0x01;
	}
	if (var->type) {
		TypeInfo* valType = var->TypeCheck(parser);

		if (valType != (TypeInfo*)0x01 && valType == var->type){
			return var->type;
		}
	}

	return (TypeInfo*)0x01;
}

Assignment::~Assignment(){
	BNS_SAFE_DELETE(var);
	BNS_SAFE_DELETE(val);
}

void FunctionCall::AddByteCode(BNVM& vm) {
	AddDebugInfo(vm, file, line);

	int externFuncIndex = -1;
	bool isExternFunc = vm.externFuncIndices.LookUp(func->name, &externFuncIndex);

	for (int i = args.count - 1; i >= 0; i--) {
		args.data[i]->AddByteCode(vm);

		// HACK: If we're passing an internal string to an external function, 
		// we convert it to a valid char*
		// TODO: This doesn't work for strings in structs...
		if (isExternFunc && args.data[i]->type->typeName == "string") {
			vm.code.PushBack(I_INTERNSTRTOEXTERN);
		}
	}

	BuiltinFunc builtinFunc;
	if (isExternFunc) {
		IntLiteral lit;
		lit.value = externFuncIndex;
		lit.AddByteCode(vm);

		vm.code.PushBack(I_EXTERNF);
	}
	else if (FindBuiltinFunc(func->name, &builtinFunc)) {
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

TypeInfo* FunctionCall::TypeCheck(BNVParser& parser) {
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
	AddDebugInfo(vm, file, line);
	for (int i = (type->size - 1) / 4 * 4; i >= 0; i -= 4) {
		IntLiteral lit;
		lit.value = regOffset + i;
		lit.AddByteCode(vm);
		if (isGlobal) {
			vm.code.PushBack(I_LOADG);
		}
		else {
			vm.code.PushBack(I_LOADI);
		}
	}
}

TypeInfo* VariableAccess::TypeCheck(BNVParser& parser) {
	if (regOffset == -1) {
		if (TypeInfo* varType = parser.GetVariableType(varName)) {
			type = varType;
			if (isGlobal) {
				regOffset = parser.GetGlobalVariableOffset(varName);
			}
			else {
				regOffset = parser.GetVariableOffset(varName);
			}

			ASSERT_MSG(regOffset >= 0, "Variable '%.*s' had an error when determining its offset.", varName.length, varName.start);

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

TypeInfo* FieldAccess::TypeCheck(BNVParser& parser) {
	TypeInfo* info = var->TypeCheck(parser);

	if (info == (TypeInfo*)0x01) {
		return info;
	}

	int fieldOffset = 0;
	TypeInfo* fieldType = (TypeInfo*)0x01;
	StructDef* def = parser.GetStructDef(var->type->typeName);
	for (int i = 0; i < def->fields.count; i++) {
		if (def->fields.data[i].name == fieldName) {
			fieldType = def->fields.data[i].type;
			type = fieldType;
			break;
		}
		else {
			fieldOffset += def->fields.data[i].type->size;
		}
	}

	regOffset = var->regOffset + fieldOffset;

	return fieldType;
}

void FuncDef::AddByteCode(BNVM& vm) {
	vm.functionPointers.Insert(name, vm.code.count);

	int index = 0;
	for (int i = 0; i < params.count; i++) {
		int paramSize = params.data[i].type->size;

		for (int j = 0; j < paramSize; j += 4) {
			IntLiteral lit;
			lit.value = index;
			lit.AddByteCode(vm);

			vm.code.PushBack(I_STOREI);

			index += 4;
		}
	}

	Scope::AddByteCode(vm);
}

TypeInfo* FuncDef::TypeCheck(BNVParser& parser) {
	return Scope::TypeCheck(parser);
}

TypeInfo* Scope::TypeCheck(BNVParser& parser) {
	for (int i = 0; i < statements.count; i++) {
		if (statements.data[i]->TypeCheck(parser) == (TypeInfo*)0x01) {
			return (TypeInfo*)0x01;
		}
	}

	return nullptr;
}

TypeInfo* IfStatement::TypeCheck(BNVParser& parser) {
	TypeInfo* intInfo;
	parser.definedTypes.LookUp("int", &intInfo);
	if (check->TypeCheck(parser) != intInfo) {
		return (TypeInfo*)0x01;
	}

	return Scope::TypeCheck(parser);
}

void BNVParser::AddByteCode(BNVM& vm) {
	vm.externFuncIndices = externFuncNames;

	// TODO: This really shouldnt be a run-time thing
	// Modify AddByteCode to also take a pointer to this BNVParser?
	vm.stringOffsets = stringOffsets;

	for (int i = 0; i < stringOffsets.count; i++) {
		SubString val = stringOffsets.data[i].substr;
		ASSERT(stringOffsets.data[i].offset == vm.code.count);

		int goal = vm.code.count + val.length + 1;
		vm.code.EnsureCapacity(goal);
		for (int j = 0; j < val.length; j++) {
			if (val.start[j] == '\\') {
				switch (val.start[j + 1]) {
				case '\\': { vm.code.PushBack('\\');} break;
				case 'n':  {vm.code.PushBack('\n'); } break;
				case 'r':  {vm.code.PushBack('\r'); } break;
				case 't':  {vm.code.PushBack('\t'); } break;
				case 'b':  {vm.code.PushBack('\b'); } break;
				case '"':  {vm.code.PushBack('"');  } break;
				default: { printf("\nWarning: Incorrect escape sequence: '%.*s'\n", 2, &val.start[j]);} break;
				}

				j++;
			}
			else {
				vm.code.PushBack(val.start[j]);
			}
		}

		for (int j = vm.code.count; j < goal; j++) {
			vm.code.PushBack('\0');
		}
	}

	for (int i = 0; i < funcDefs.count; i++) {
		BuiltinFunc builtinFunc;
		if (!FindBuiltinFunc(funcDefs.data[i]->name, &builtinFunc)) {
			funcDefs.data[i]->AddByteCode(vm);
		}
	}

	for (int i = 0; i < globalVars.count; i++) {
		SubString name = globalVars.data[i].name;
		vm.inst.globalVarRegs.Insert(name, GetGlobalVariableOffset(name));
	}

	vm.inst.globalVarSize = GetGlobalVarSize();
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

struct Vector3VM {
	float x;
	float y;
	float z;

	Vector3VM() = default;

	Vector3VM(float _x, float _y, float _z) {
		x = _x;
		y = _y;
		z = _z;
	}
};

#include <math.h>

void mySin(TempStack* stk) {
	float val = stk->Pop<float>();
	float ret = sinf(val);
	stk->Push(ret);
}

void mySub(TempStack* stk) {
	float a = stk->Pop<float>();
	float b = stk->Pop<float>();
	float ret = a - b;
	stk->Push(ret);
}

void myDot(TempStack* stk) {
	Vector3VM a = stk->Pop<Vector3VM>();
	Vector3VM b = stk->Pop<Vector3VM>();
	float ret = a.x*b.x + a.y*b.y + a.z*b.z;
	stk->Push(ret);
}

void FlipCase(TempStack* stk) {
	char* str = stk->Pop<char*>();
	while (*str) {
		if ((*str >= 'a' && *str <= 'z' )
		 || (*str >= 'A' && *str <= 'Z')) {
			*str ^= ('A' ^ 'a');
		}

		str++;
	}
}

void ShowFileContentsAtLine(char* file, int lineNum) {
	char* cursor = file;

	int currLine = 1;
	while (*cursor) {
		int nextLine = FindChar(cursor, '\n');

		printf("%s%3d:%.*s\n", (currLine == lineNum ? ">>>>" : "    "), currLine, (nextLine < 0 ? StrLen(cursor) : nextLine), cursor);

		if (nextLine < 0) {
			break;
		}

		cursor += (nextLine + 1);
		currLine++;
	}
}

// TODO: We previously were running this in a shell script, and could redirect the printed output to a file,
// then diff it as another test.
// Now that we run tests in a single process, we'll need to steup BNVM to be able to redirect stdin/stdout to files
// in c++, and do the diff in c++ as well.
CREATE_TEST_CASE("BNVParser full system") {
	BNVParser parser;

	parser.ParseFile("parserTest.bnv");

	// These ended up just being noise...
	//ASSERT(parser.funcDefs.count == 25);
	//ASSERT(parser.funcDefs.data[24]->name == "main");

	BNVM vm;
	parser.AddByteCode(vm);

	vm.RegisterExternFunc("sinf", mySin);
	vm.RegisterExternFunc("subtract", mySub);
	vm.RegisterExternFunc("DotProductExt", myDot);
	vm.RegisterExternFunc("FlipCase", FlipCase);

	vm.Execute("main");

	ASSERT((vm.ExecuteTyped<float, float>("sinTest", 2.3f) == sinf(2.3f)));
	ASSERT((vm.ExecuteTyped<float, float>("subtractTest", -4.0f) == 5.0f));
	ASSERT((vm.ExecuteTyped<Vector3VM, float>("DotProductExtTest", Vector3VM(1.0f, 3.0f, 2.0f)) == 6.0f));

	ASSERT((vm.ExecuteTyped<int, int>("Factorial", 5) == 120));
	ASSERT((vm.ExecuteTyped<int, int>("Factorial", 0) == 1));
	ASSERT((vm.ExecuteTyped<int, int>("IsPrime", 2) == 1));
	ASSERT((vm.ExecuteTyped<int, int>("IsPrime", 5) == 1));
	ASSERT((vm.ExecuteTyped<int, int>("IsPrime", 6) == 0));
	ASSERT((vm.ExecuteTyped<int, int>("IsPrime", 17) == 1));
	ASSERT((vm.ExecuteTyped<int, int>("IsPrime", 117) == 0));
	ASSERT((vm.ExecuteTyped<int, int>("IsPrime", 997) == 1));
	ASSERT((vm.ExecuteTyped<int, int>("IsPrime", 1024) == 0));
	ASSERT((vm.ExecuteTyped<int, int>("IsPrime", 1024) == 0));
	ASSERT((vm.ExecuteTyped<Vector3VM, float>("VectorLengthSqr", Vector3VM(3.0f, 4.0f, 12.0f)) == 169.0f));

	int globalIntegerOffset = -1;
	vm.inst.globalVarRegs.LookUp("globalInteger", &globalIntegerOffset);

	ASSERT(globalIntegerOffset >= 0);

	for (int i = 0; i < 68000; i += 100) {
		vm.ExecuteTyped<int>("SetGlobalInteger", i);
		ASSERT(vm.GetGlobalVariableValueByOffset<int>(globalIntegerOffset) == i);
	}

	for (int i = 0; i < 60; i++) {
		vm.ExecuteTyped<int>("SetGlobalInteger", i);
		ASSERT((vm.ExecuteTyped<int, int>("IsPrime", 1024) == 0));
		ASSERT((vm.ExecuteTyped<int, int>("Factorial", 7) == 5040));
		ASSERT(vm.GetGlobalVariableValueByOffset<int>(globalIntegerOffset) == i);
	}

	int globalVecOffset = -1;
	vm.inst.globalVarRegs.LookUp("globalVec", &globalVecOffset);

	ASSERT(globalVecOffset >= 0);

	vm.ExecuteTyped<float>("SetGlobalVecX", 12.3f);
	ASSERT((vm.ExecuteTyped<int, int>("IsPrime", 1024) == 0));
	ASSERT(vm.GetGlobalVariableValueByOffset<float>(globalVecOffset) == 12.3f);

	ASSERT((vm.ExecuteTyped<int, int>("IsPrime", 1024) == 0));
	vm.ExecuteTyped<Vector3VM>("SetGlobalVec", Vector3VM(43.5f, -12.3f, 14.4f));
	ASSERT((vm.ExecuteTyped<int, int>("IsPrime", 1024) == 0));
	{
		Vector3VM globalVec = vm.GetGlobalVariableValueByOffset<Vector3VM>(globalVecOffset);
		ASSERT(globalVec.x == 43.5f);
		ASSERT(globalVec.y == -12.3f);
		ASSERT(globalVec.z == 14.4f);
	}

	vm.ExecuteTyped<float>("SetGlobalVecX", 15.3f);
	ASSERT((vm.ExecuteTyped<int, int>("IsPrime", 1024) == 0));
	{
		Vector3VM globalVec = vm.GetGlobalVariableValueByOffset<Vector3VM>(globalVecOffset);
		ASSERT(globalVec.x == 15.3f);
		ASSERT(globalVec.y == -12.3f);
		ASSERT(globalVec.z == 14.4f)
	}

	printf("================\n");
	vm.ExecuteTyped<Vector3VM>("PrintVector", Vector3VM(1.2f, 2.3f, 3.4f));

	{
		BNVParser relIncludeParser;

		relIncludeParser.ParseFile("relIncludeTest.bnv");

		BNVM relVm;
		relIncludeParser.AddByteCode(relVm);

		ASSERT((relVm.ExecuteTyped<int, int>("main", 17) == 3));
	}

	vm.ExecuteTyped<int>("SetGlobalInteger", 10);
	ASSERT((vm.GetGlobalVariableValue<int>("globalInteger") == 10));
	ASSERT((vm.inst.GetGlobalVariableValue<int>("globalInteger") == 10));

	BNVMInstance otherInstance;
	vm.InitNewInst(&otherInstance);

	otherInstance.ExecuteTyped<int>("SetGlobalInteger", 99);
	ASSERT((vm.GetGlobalVariableValue<int>("globalInteger") == 10));
	ASSERT((vm.inst.GetGlobalVariableValue<int>("globalInteger") == 10));

	ASSERT((otherInstance.GetGlobalVariableValue<int>("globalInteger") == 99));

	vm.ExecuteTyped<int>("SetGlobalInteger", -56);
	ASSERT((vm.GetGlobalVariableValue<int>("globalInteger") == -56));
	ASSERT((otherInstance.GetGlobalVariableValue<int>("globalInteger") == 99));

	ASSERT((otherInstance.ExecuteTyped<int, int>("Factorial", 5) == 120));
	ASSERT((otherInstance.ExecuteTyped<int, int>("Factorial", 0) == 1));
	ASSERT((otherInstance.ExecuteTyped<int, int>("IsPrime", 2) == 1));
	ASSERT((otherInstance.ExecuteTyped<int, int>("IsPrime", 5) == 1));

	{
		BNVMInstance thirdInstance = otherInstance;
		vm.InitNewInst(&thirdInstance);

		thirdInstance.ExecuteTyped<int>("SetGlobalInteger", 88);
		ASSERT((vm.GetGlobalVariableValue<int>("globalInteger") == -56));
		ASSERT((otherInstance.GetGlobalVariableValue<int>("globalInteger") == 99));
		ASSERT((thirdInstance.GetGlobalVariableValue<int>("globalInteger") == 88));

		ASSERT((thirdInstance.ExecuteTyped<int, int>("Factorial", 5) == 120));
		ASSERT((thirdInstance.ExecuteTyped<int, int>("Factorial", 0) == 1));
		ASSERT((thirdInstance.ExecuteTyped<int, int>("IsPrime", 2) == 1));
		ASSERT((thirdInstance.ExecuteTyped<int, int>("IsPrime", 5) == 1));
	}

	vm.WriteByteCodeToFile("parserTest.bnb");

	BNVM otherVm;
	otherVm.ReadByteCodeFromFile("parserTest.bnb");
	otherVm.RegisterExternFunc("sinf", mySin);
	otherVm.RegisterExternFunc("subtract", mySub);
	otherVm.RegisterExternFunc("DotProductExt", myDot);

	ASSERT((otherVm.ExecuteTyped<int, int>("Factorial", 5) == 120));
	ASSERT((otherVm.ExecuteTyped<int, int>("Factorial", 0) == 1));
	ASSERT((otherVm.ExecuteTyped<int, int>("IsPrime", 2) == 1));
	ASSERT((otherVm.ExecuteTyped<int, int>("IsPrime", 5) == 1));

	otherVm.ExecuteTyped<int>("SetGlobalInteger", 99);
	ASSERT((otherVm.GetGlobalVariableValue<int>("globalInteger") == 99));

	{
		BNVParser parser2;

		parser2.ParseFile("prime.bnv");

		BNVM vm2;
		parser2.AddByteCode(vm2);


		vm2.WriteByteCodeToFile("prime.bnb");
	}

	// Debugger testing.
	/*
	{
		BNVParser parser2;
		parser2.generateDebugInfo = true;

		parser2.ParseFile("prime.bnv");

		BNVM vm2;
		parser2.AddByteCode(vm2);


		vm2.WriteByteCodeToFile("prime_dbg.bnb");

		vm2.inst.debugState = DS_StepNext;

		StringMap<String> fileContentsMap;

		//printf("----------------\n");
		BNVMReturnReason reason = vm2.Execute("main");
		while (reason != RR_Done) {
			int fileIdx = BNVM_UNPACK_FILE(vm2.inst.currentFileLine);
			int line = BNVM_UNPACK_LINE(vm2.inst.currentFileLine);

			String file;
			bool found = false;
			// TODO: Optimise this, make it just an index rather than a map to id's
			for (int i = 0; i < vm2.debugFileIndices.count; i++) {
				if (vm2.debugFileIndices.values[i] == fileIdx) {
					file = vm2.debugFileIndices.names[i];
					found = true;
				}
			}

			ASSERT(found);

			String contents;
			if (!fileContentsMap.LookUp(file, &contents)) {
				contents = ReadStringFromFile(file.string);
				fileContentsMap.Insert(file, contents);
			}

			ShowFileContentsAtLine(contents.string, line);

			REPL:
			printf("\ns - step, c - continue, q - quit v# - print val [# = stack offset] t# - print trace [# - frame]\n");
			char response[16];
			scanf("%10s", response);
			if (response[0] == 'c') {
				vm2.inst.debugState = DS_Continue;
			}
			else if (response[0] == 's') {
				vm2.inst.debugState = DS_StepNext;
			}
			else if (response[0] == 'q') {
				break;
			}
			else if (response[0] == 'v') {
				int offset = Atoi(&response[1]);
				int* val = vm2.inst.varStack.Access<int>(offset);
				printf("Val at [%d]: %d\n", offset, *val);
				goto REPL;
			}
			else if (response[0] == 't') {
				int frame = Atoi(&response[1]);
				int index = vm2.inst.callStack.stackMem.count - 8 * (frame + 1);
				int* frameData = (int*)&vm2.inst.callStack.stackMem.data[index];

				int closestPointerUnderFrame = 0;
				char* closestFuncName = "<UNKNOWN>";
				for (int i = 0; i < vm2.functionPointers.count; i++) {
					int funcStart = vm2.functionPointers.values[i];
					if (funcStart > closestPointerUnderFrame && funcStart <= *frameData) {
						closestPointerUnderFrame = funcStart;
						closestFuncName = vm2.functionPointers.names[i].string;
					}
				}

				printf("Frame %d: %s+%d\n", frame, closestFuncName, *frameData - closestPointerUnderFrame);
				goto REPL;
			}


			reason = vm2.ExecuteInternal();
		}
		//printf("----------------\n");
	}
	*/

	return 0;
}




#endif

