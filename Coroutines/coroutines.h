#ifndef COROUTINES_H
#define COROUTINES_H

#pragma once

struct MetaTypeInfo{
	int pointerLevel;
	SubString arrayCount;
	SubString typeName;
	SubString genericParam;

	bool isConst;
	bool isReference;

	int tokenStartIndex;
	int tokenEndIndex;

	MetaTypeInfo() {
		pointerLevel = 0;
		tokenStartIndex = 0;
		tokenEndIndex = 0;
		isConst = false;
		isReference = false;
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

	Vector<MetaVarDecl> closureVars;

	MetaFuncDef* parent;
	Vector<MetaFuncDef> localFuncDefs;
	int headerStartIndex;
	int bodyStartIndex;
	int endIndex;

	MetaFuncDef() {
		parent = nullptr;
	}
};

int ParseFuncHeader(const Vector<SubString>& tokens, int startingIndex, MetaFuncDef* outFuncDef);

int ParseCoroutineFuncDef(const Vector<SubString>& tokens, int startingIndex, MetaFuncDef* outFuncDef);

void SerializeVarDecl(MetaTypeInfo* info, const char* varName, int varNameLength, char* buffer, int buffLength);

void ParseFunctionsFromTokens(const Vector<SubString>* tokens, Vector<MetaFuncDef>* outDefs);

bool IsVarInScope(const SubString& token, const MetaFuncDef* funcDef, MetaVarDecl* outDecl = nullptr);

bool FuncHasClosure(MetaFuncDef* def, SubString tok);

inline bool IsAlpha(char c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

inline bool IsIdentifier(const SubString token) {
	return token.length > 0 && (IsAlpha(token.start[0]) || token.start[0] == '_');
}

#endif
