#include "../lexer.h"

//--------------------
// metagen

struct MetaTypeInfo{
	int pointerLevel;
	SubString arrayCount;
	SubString typeName;
	SubString genericParam;

	bool isConst;

	int tokenStartIndex;
	int tokenEndIndex;

	MetaTypeInfo() {
		pointerLevel = 0;
		tokenStartIndex = 0;
		tokenEndIndex = 0;
		isConst = false;
	}
};

struct MetaVarDecl{
	int tokenIndex;
	SubString name;
	MetaTypeInfo type;

	MetaVarDecl() {
		tokenIndex = 0;
	}
};

struct MetaFuncDef{
	MetaTypeInfo retType;
	SubString name;
	Vector<MetaVarDecl> params;
	Vector<MetaVarDecl> localVars;
};

void SerializeVarDecl(MetaTypeInfo* info, const char* varName, int varNameLength, char* buffer, int buffLength){
	if (info->isConst) {
		buffer += snprintf(buffer, buffLength, "const ");
	}

	buffer += snprintf(buffer, buffLength, "%.*s", info->typeName.length, info->typeName.start);
	
	if (info->genericParam.start != nullptr) {
		buffer += snprintf(buffer, buffLength, "<%.*s>", info->genericParam.length, info->genericParam.start);
	}
	
	for (int i = 0; i < info->pointerLevel; i++) {
		buffer += snprintf(buffer, buffLength, "*");
	}

	buffer += snprintf(buffer, buffLength, " %.*s", varNameLength, varName);

	if (info->arrayCount.start != nullptr) {
		buffer += snprintf(buffer, buffLength, "[%.*s]", info->arrayCount.length, info->arrayCount.start);
	}
}

void SerializeTypeName(MetaTypeInfo* info, char* buffer, int buffLength){
	// TODO: do
}

//-------------------------
// runtime

//-------------------------
// example usage
#if 0

#define BNS_YIELD()
#define BNS_YIELD_FROM(f) f##_Func()

/*[Coroutine]*/
void loopThroughFlts(const float* arr, int count) {
	float total = 0;
	for (int i = 0; i < count; i++) {
		total += arr[i];
		printf("num: %f, total: %f\n", arr[i], total);
		BNS_YIELD();
	}
}

/*[Coroutine]*/
void loopThroughIVec(const Vector<int>* iVec) {
	for (int i = 0; i < iVec->count; i++) {
		printf("idx: %d, val: %d\n", i, iVec->data[i]);
		BNS_YIELD();
	}
}

//-------------------------------
// example gen
struct loopThroughFlts_Data{
	int _case;
	const float* _arr_5;
	int _count_8;
	int _i_14;
	
	loopThroughFlts_Data(float* arrParam, int countParam){
		_arr_5 = arrParam;
		_count_8 = countParam;
		_case = 0;
	}
};

CoroutineResult loopThroughFlts_Func(void* ptr){
	loopThroughFlts_Data* data = (loopThroughFlts_Data*)ptr;
	switch (data->_case) {
	case 0:
		for (data->_i_14 = 0; data->_i_14 < data->_count_8; data->_i_14++) {
			printf("%f\n", data->_arr_5[data->_i_14]);
			data->_case = 1;
			return CoroutineResult::CR_Yield;
	case 1:;
		}
	}

	return CoroutineResult::CR_Return;
}
#endif

//-------------------------------

inline bool IsAlpha(char c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool IsIdentifier(const SubString& token) {
	return token.length > 0 && (IsAlpha(token.start[0]) || token.start[0] == '_');
}

int ParseTypeName(const Vector<SubString>& tokens, int startingIndex, MetaTypeInfo* outType) {
	int index = startingIndex;
	if (index < tokens.count && tokens.Get(index) == "const") {
		outType->isConst = true;
		index++;
	}

	if (!IsIdentifier(tokens.Get(index))) {
		return startingIndex;
	}

	if (index < tokens.count) {
		outType->typeName = tokens.Get(index);
		index++;
	}

	if (index < tokens.count - 2 && tokens.Get(index) == "<" && tokens.Get(index + 2) == ">") {
		outType->genericParam = tokens.Get(index + 1);
		index += 3;
	}

	while (index < tokens.count && tokens.Get(index) == "*") {
		outType->pointerLevel++;
		index++;
	}

	outType->tokenStartIndex = startingIndex;
	outType->tokenEndIndex = index - 1;

	return index;
}

int ParseVarDecl(const Vector<SubString>& tokens, int startingIndex, MetaVarDecl* outVar) {
	int index = ParseTypeName(tokens, startingIndex, &outVar->type);

	if (index < tokens.count) {
		outVar->name = tokens.Get(index);
		outVar->tokenIndex = index;
		index++;
	}

	if (index < tokens.count - 2 && tokens.Get(index) == "[" && tokens.Get(index + 2) == "]"){
		outVar->type.arrayCount = tokens.Get(index + 1);
		index += 3;
	}

	return index;
}

int ParseFuncHeader(const Vector<SubString>& tokens, int startingIndex, MetaFuncDef* outFuncDef) {
	int index = startingIndex;

	index = ParseTypeName(tokens, index, &outFuncDef->retType);

	if (index < tokens.count) {
		outFuncDef->name = tokens.Get(index);
		index++;
	}

	if (tokens.Get(index) != "(") {
		return startingIndex;
	}
	else {
		index++;
	}

	int closeParen = index;
	while (closeParen < tokens.count && tokens.Get(closeParen) != ")") {
		closeParen++;
	}

	if (closeParen >= tokens.count - 1 || tokens.Get(closeParen + 1) != "{") {
		return startingIndex;
	}

	for (; index < closeParen; index++) {
		MetaVarDecl param;
		index = ParseVarDecl(tokens, index, &param);
		if (tokens.Get(index) != "," && index != closeParen) {
			return startingIndex;
		}

		outFuncDef->params.PushBack(param);
	}

	return index;
}

bool IsVarInScope(const SubString& token, const MetaFuncDef* funcDef) {
	for (int i = 0; i < funcDef->params.count; i++) {
		if (funcDef->params.Get(i).name == token) {
			return true;
		}
	}

	for (int i = 0; i < funcDef->localVars.count; i++) {
		if (funcDef->localVars.Get(i).name == token) {
			return true;
		}
	}

	return false;
}

const MetaVarDecl* GetDeclForVar(const SubString& token, const MetaFuncDef* funcDef) {
	for (int i = 0; i < funcDef->params.count; i++) {
		if (funcDef->params.Get(i).name == token) {
			return &funcDef->params.Get(i);
		}
	}

	for (int i = 0; i < funcDef->localVars.count; i++) {
		if (funcDef->localVars.Get(i).name == token) {
			return &funcDef->localVars.Get(i);
		}
	}

	return nullptr;
}

int ParseCoroutineFuncDef(const Vector<SubString>& tokens, int startingIndex, MetaFuncDef* outFuncDef) {
	int index = startingIndex;

	index = ParseFuncHeader(tokens, startingIndex, outFuncDef);

	int braceCount = 1;
	int endIndex = index + 1;
	while (endIndex < tokens.count && braceCount > 0) {
		if (tokens.Get(endIndex) == "{") {
			braceCount++;
		}
		else if (tokens.Get(endIndex) == "}") {
			braceCount--;
		}

		endIndex++;
	}

	ASSERT(braceCount == 0);

	// Skip past opening brace
	index++;

	while (index < endIndex) {
		if (IsIdentifier(tokens.Get(index)) && !IsVarInScope(tokens.Get(index), outFuncDef)
			&& index < endIndex - 1 && tokens.Get(index + 1) != "(") {
			MetaVarDecl varDecl;
			index = ParseVarDecl(tokens, index, &varDecl);

			if (tokens.Get(index) == "=" || tokens.Get(index) == ";") {
				outFuncDef->localVars.PushBack(varDecl);
			}
		}
		else if (tokens.Get(index) == "BNS_YIELD_FROM" || tokens.Get(index) == "BNS_YIELD_FROM_ARGS"){
			if (index < tokens.count - 2){
				SubString callee = tokens.Get(index + 2);
				StringStackBuffer<256> calleeDataName("%.*s_Data", callee.length, callee.start);
				
				// We need a substring for this, but it doesn't actually appear in the function body's text
				String calleDataString = String(calleeDataName.buffer);
				SubString calleDataSubstr = calleDataString.GetSubString(0, calleDataString.GetLength());
				
				MetaVarDecl varDecl;
				varDecl.type.typeName = calleDataSubstr;
				// No type name inline, so don't remove any tokens.
				varDecl.type.tokenStartIndex = 0;
				varDecl.type.tokenEndIndex = -1;
				// Might as well re-use this, so we have an informative name.  It'll have _0 appended to it.
				varDecl.name = calleDataSubstr;
				outFuncDef->localVars.PushBack(varDecl);
			}
			index++;
		}
		else {
			while (index < endIndex) {
				if (tokens.Get(index) == ";") {
					index++;
					break;
				}
				// HACK: For loop var declaration
				// TODO: var decl in if as well?
				else if (index < endIndex - 1 && tokens.Get(index) == "for" && tokens.Get(index+1) == "(") {
					index += 2;
					break;
				}

				index++;
			}
		}
	}
	
	return endIndex;
}

template<int cap>
void GetNewFieldName(const MetaVarDecl& decl, StringStackBuffer<cap>* buffer) {
	buffer->AppendFormat(
		"_%.*s_%d",
		decl.name.length,
		decl.name.start,
		decl.tokenIndex);
}

void PrintYieldToFile(FILE* fileHandle, int yieldCount, int braceCount) {
	fprintf(fileHandle, "_data->_case = %d;\n", yieldCount);
	for (int j = 0; j < braceCount; j++) {
		fprintf(fileHandle, "\t");
	}
	fprintf(fileHandle, "return CoroutineResult::CR_Yield;\n");
	for (int j = 0; j < braceCount; j++) {
		fprintf(fileHandle, "\t");
	}
	fprintf(fileHandle, "case %d: ;\n", yieldCount);
	for (int j = 0; j < braceCount; j++) {
		fprintf(fileHandle, "\t");
	}
}

void PrintYieldFrom(FILE* fileHandle, SubString calllee, int braceCount, int* yieldCount) {
	fprintf(fileHandle, "\n");
	for (int j = 0; j < braceCount; j++) {
		fprintf(fileHandle, "\t");
	}

	fprintf(fileHandle, "while (%.*s_Func(&_data->_%.*s_Data_0) != CoroutineResult::CR_Return){\n",
		calllee.length, calllee.start,
		calllee.length, calllee.start);
	for (int j = 0; j < braceCount + 1; j++) {
		fprintf(fileHandle, "\t");
	}

	(*yieldCount)++;
	PrintYieldToFile(fileHandle, *yieldCount, braceCount + 1);
	fprintf(fileHandle, "}\n");
}

void GenerateCoroutineWrapperCode(
		const Vector<SubString> tokens, int startingIndex, int endingIndex,
		const MetaFuncDef* def, FILE* fileHandle) {

	fprintf(fileHandle, "\n//------------------------");
	fprintf(fileHandle, "\n// generated code\n\n");

	StringStackBuffer<256> structName("%.*s_Data", def->name.length, def->name.start);
	StringStackBuffer<256> funcName("%.*s_Func", def->name.length, def->name.start);

	fprintf(fileHandle, "struct %s {\n", structName.buffer);
	fprintf(fileHandle, "\tint _case;\n");

	for (int i = 0; i < def->params.count; i++) {
		StringStackBuffer<256> newFieldName;
		GetNewFieldName(def->params.Get(i), &newFieldName);

		char fieldBuffer[256];
		SerializeVarDecl(&def->params.data[i].type, newFieldName.buffer, newFieldName.length, fieldBuffer, sizeof(fieldBuffer));
		fprintf(fileHandle, "\t%s;\n", fieldBuffer);
	}

	for (int i = 0; i < def->localVars.count; i++) {
		StringStackBuffer<256> newFieldName;
		GetNewFieldName(def->localVars.Get(i), &newFieldName);

		char fieldBuffer[256];
		SerializeVarDecl(&def->localVars.data[i].type, newFieldName.buffer, newFieldName.length, fieldBuffer, sizeof(fieldBuffer));
		fprintf(fileHandle, "\t%s;\n", fieldBuffer);
	}

	// Dumb default constructor to make C++ shut up
	if (def->params.count > 0) {
		fprintf(fileHandle, "\n\t%s(){\n\t}", structName.buffer);
	}

	fprintf(fileHandle, "\n\t%s(", structName.buffer);
	for (int i = 0; i < def->params.count; i++) {
		if (i > 0) {
			fprintf(fileHandle, ", ");
		}

		StringStackBuffer<256> paramName("%.*sParam", def->params.Get(i).name.length, def->params.Get(i).name.start);
		char paramBuffer[256];
		SerializeVarDecl(&def->params.data[i].type, paramName.buffer, paramName.length, paramBuffer, sizeof(paramBuffer));
		fprintf(fileHandle, "%s", paramBuffer);

	}
	fprintf(fileHandle, ") {\n");

	fprintf(fileHandle, "\t\t_case = 0;\n");

	for (int i = 0; i < def->params.count; i++) {
		StringStackBuffer<256> paramName("%.*sParam", def->params.Get(i).name.length, def->params.Get(i).name.start);
		StringStackBuffer<256> newFieldName;
		GetNewFieldName(def->params.Get(i), &newFieldName);
		fprintf(fileHandle, "\t\t%s = %s;\n", newFieldName.buffer, paramName.buffer);
	}

	fprintf(fileHandle, "\t}\n");

	fprintf(fileHandle, "};\n");

	fprintf(fileHandle, "\nCoroutineResult %.*s_Func(void* ptr){\n", def->name.length, def->name.start);
	fprintf(fileHandle, "\t%.*s_Data* _data = (%.*s_Data*)ptr;\n", 
		def->name.length, def->name.start, def->name.length, def->name.start);
	fprintf(fileHandle, "\tswitch (_data->_case) {\n");
	fprintf(fileHandle, "\tcase 0:\n\t");

	while (tokens.Get(startingIndex) != "{") {
		startingIndex++;
	}

	// Skip past the first brace, since we're adding it above
	startingIndex++;

	// We subtract 1 to skip the closing brace. We add it manually later
	Vector<SubString> functionTokens = tokens.GetSubVector(startingIndex, endingIndex - startingIndex - 1);
	int offset = 0;

	for (int i = 0; i < def->localVars.count; i++) {
		int typeTokStart = def->localVars.Get(i).type.tokenStartIndex;
		int typeTokLen = def->localVars.Get(i).type.tokenEndIndex - def->localVars.Get(i).type.tokenStartIndex + 1;

		int typeTokRemap = typeTokStart - startingIndex - offset;

		for (int i = 0; i < typeTokLen; i++) {
			functionTokens.Remove(typeTokRemap);
			offset++;
		}
	}

	int yieldCount = 0;
	int braceCount = 1;
	int parenCount = 0;
	bool inYieldFromArgs = false;
	SubString yieldFromArgsCallee;
	for (int i = 0; i < functionTokens.count; i++) {
		SubString tok = functionTokens.Get(i);
		if (const MetaVarDecl* decl = GetDeclForVar(tok, def)) {
			StringStackBuffer<256> fieldName;
			GetNewFieldName(*decl, &fieldName);
			fprintf(fileHandle, "_data->%s ", fieldName.buffer);
		}
		else if (tok == "BNS_YIELD") {
			yieldCount++;

			PrintYieldToFile(fileHandle, yieldCount, braceCount);

			i += 2;
		}
		else if (tok == "BNS_YIELD_FROM") {
			SubString calleName = functionTokens.Get(i + 2);
			i += 3;
		}
		else if (tok == "BNS_YIELD_FROM_ARGS") {
			ASSERT(parenCount == 0);
			SubString calleeName = functionTokens.Get(i + 2);

			fprintf(fileHandle, "_data->_%.*s_Data_0 = %.*s_Data(", 
				calleeName.length, calleeName.start,
				calleeName.length, calleeName.start);

			yieldFromArgsCallee = calleeName;
			inYieldFromArgs = true;

			i += 3;
			parenCount++;
		}
		else {
			bool newLine = false;
			if (tok == "{") {
				braceCount++;
				newLine = true;
			}
			else if (tok == "}") {
				braceCount--;
				newLine = true;
				// HACK: Is this just for console output?
				fprintf(fileHandle, "\r");
				for (int j = 0; j < braceCount; j++) {
					fprintf(fileHandle, "\t");
				}
			}
			else if (tok == ";" && parenCount == 0) {
				newLine = true;
			}
			else if (tok == "(") {
				parenCount++;
			}
			else if (tok == ")") {
				parenCount--;

				if (parenCount == 0 && inYieldFromArgs) {
					// Get the semi colon and paren in the right place
					fprintf(fileHandle, ");");
					i++;

					PrintYieldFrom(fileHandle, yieldFromArgsCallee, braceCount, &yieldCount);
					inYieldFromArgs = false;
					
					// HACK: Don't print the paren, since we added a bunch of other stuff
					tok.length = 0;
				}
			}

			fprintf(fileHandle, "%.*s ", tok.length, tok.start);

			if (newLine) {
				fprintf(fileHandle, "\n");
				for (int j = 0; j < braceCount; j++) {
					fprintf(fileHandle, "\t");
				}
			}
		}
	}

	fprintf(fileHandle, "\n\tdefault:\n\tbreak;}");
	fprintf(fileHandle, "\n\t_data->_case = -1;");
	fprintf(fileHandle, "\n\treturn CoroutineResult::CR_Return; \n}\n");

	fprintf(fileHandle, "//------------------------\n");
}

void ParseCoroutinesInFiles(const char** files, int fileCount, FILE* fileHandle) {
	fprintf(fileHandle, "#ifndef COROUTINEGEN_H\n");
	fprintf(fileHandle, "#define COROUTINEGEN_H\n");
	fprintf(fileHandle, "#include \"coroutine_runtime.h\"\n\n");

	Vector<SubString> coroutineNames;
	for (int f = 0; f < fileCount; f++) {
		String fileContents = ReadStringFromFile(files[f]);
		Vector<SubString> lexedFile = LexString(fileContents);

		for (int i = 0; i < lexedFile.count; i++) {
			if (lexedFile.Get(i) == "/*[") {
				if (i < lexedFile.count - 2) {
					if (lexedFile.Get(i + 1) == "Coroutine" && lexedFile.Get(i + 2) == "]*/") {
						MetaFuncDef funcDef;
						int startIndex = i + 3;
						i = ParseCoroutineFuncDef(lexedFile, i + 3, &funcDef);

						GenerateCoroutineWrapperCode(lexedFile, startIndex, i, &funcDef, fileHandle);
						coroutineNames.PushBack(funcDef.name);

						// Since we increment i for the loop
						i--;
					}
				}
			}
		}
	}

	if (coroutineNames.count > 0) {
		fprintf(fileHandle, "\n#define COROT_MAX(a, b) ((a) > (b) ? (a) : (b))\n");
		fprintf(fileHandle, "const int maxCoroutineDataSize = \n");
		for (int i = 0; i < coroutineNames.count; i++) {
			fprintf(fileHandle, "\tCOROT_MAX(sizeof(%.*s_Data), \n", 
				coroutineNames.Get(i).length,
				coroutineNames.Get(i).start);
		}

		fprintf(fileHandle, "\t0");

		for (int i = 0; i < coroutineNames.count; i++) {
			fprintf(fileHandle, ")");
		}

		fprintf(fileHandle, ";\n\n");
		fprintf(fileHandle, "\n#undef COROT_MAX\n");
	}

	fprintf(fileHandle, "\n#endif\n");
}

#if defined (COROUTINE_MAIN)

int main(int argc, char** argv){
	
	
	FILE* fileHandle = fopen("coroutine_gen.h", "wb");

	const char* files[] = {"coroutines_example_use.cpp"};

	ParseCoroutinesInFiles(files, BNS_ARRAY_COUNT(files), fileHandle);

	fclose(fileHandle);

	return 0;
}

#endif