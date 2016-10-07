#ifndef COROUTINES_H
#define COROUTINES_H

#pragma once

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
	SubString nameSpace;
	Vector<MetaVarDecl> params;
	Vector<MetaVarDecl> localVars;
};

int ParseCoroutineFuncDef(const Vector<SubString>& tokens, int startingIndex, MetaFuncDef* outFuncDef);

void SerializeVarDecl(MetaTypeInfo* info, const char* varName, int varNameLength, char* buffer, int buffLength);

void ParseFunctionsFromTokens(const Vector<SubString>& tokens, Vector<MetaFuncDef>* outDefs);

#endif
