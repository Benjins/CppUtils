#ifndef BNVPARSER_H
#define BNVPARSER_H

#pragma once

#include "strings.h"
#include "stringmap.h"
#include "vector.h"

struct BNVToken{
	SubString substr;
	String file;
	int line;
};

struct BNVM;
struct BNVParser;
struct TypeInfo;
struct Value;
struct FuncDef;

struct Statement{
	virtual void AddByteCode(BNVM& vm) = 0;
	virtual TypeInfo* TypeCheck(const BNVParser& parser) = 0;
};

struct ReturnStatement : Statement{
	Value* retVal;
	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);
};

struct Scope : Statement{
	Vector<Statement*> statements;
	
	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);
};

struct IfStatement : Scope{
	Value* check;
	
	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);
};

struct WhileStatement : IfStatement{
	virtual void AddByteCode(BNVM& vm);
};


struct Value : Statement{
	TypeInfo* type;
	virtual void AddByteCode(BNVM& vm) = 0;
	virtual TypeInfo* TypeCheck(const BNVParser& parser) = 0;
};

struct IntLiteral : Value {
	int value;

	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);
};

struct FloatLiteral : Value {
	float value;

	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);
};

struct BinaryOp : Value{
	Value* lVal;
	Value* rVal;
	SubString op;
	
	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);
};

struct UnaryOp : Value{
	Value* val;
	SubString op;
	
	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);
};

struct Assignment : Statement{
	SubString varName;
	int regIndex;
	Value* val;

	Assignment() {
		regIndex = -1;
	}
	
	virtual TypeInfo* TypeCheck(const BNVParser& parser);
	virtual void AddByteCode(BNVM& vm);
};

struct FunctionCall : Value{
	FuncDef* func;
	int stackFrameOffset;
	
	Vector<Value*> args;
	
	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);
};

struct VariableAccess : Value {
	SubString varName;
	int regOffset;
	TypeInfo* info;

	VariableAccess() {
		regOffset = -1;
	}

	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);
};


struct VarDecl{
	SubString name;
	TypeInfo* type;
};

struct FuncDef : Scope{
	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);
	
	SubString name;
	Vector<VarDecl> params;
	TypeInfo* retType;
};

struct StructDef{
	SubString name;
	Vector<VarDecl> fields;
};

struct TypeInfo{
	SubString typeName;
	int size;
};

struct BNVParser{
	Vector<BNVToken> toks;
	int cursor;
	Vector<int> cursorFrames;
	
	StringMap<TypeInfo*> definedTypes;
	
	Vector<StructDef*> structDefs;
	Vector<FuncDef*> funcDefs;
	
	Scope* currScope;
	
	Vector<VarDecl> varsInScope;
	Vector<int> varFrames;

	BNVParser();
	
	~BNVParser(){
		for(int i = 0; i < structDefs.count; i++){
			delete structDefs.data[i];
		}
		for(int i = 0; i < funcDefs.count; i++){
			delete funcDefs.data[i];
		}
	}
	
	void ParseFile(const char* filename);

	void AddByteCode(BNVM& vm);
	bool TypeCheck();
	
	StructDef* ParseStructDef();
	FuncDef* ParseFuncDef();
	Statement* ParseStatement();
	ReturnStatement* ParseReturnStatement();
	IfStatement* ParseIfStatement();
	WhileStatement* ParseWhileStatement();
	Assignment* ParseAssignment();

	bool ExpectAndEatWord(const char* word){
		if(toks.data[cursor].substr == word){
			cursor++;
			return true;
		}
		else{
			return false;
		}
	}
	
	void PushCursorFrame(){
		cursorFrames.PushBack(cursor);
	}
	
	void PopCursorFrame(){
		cursor = cursorFrames.data[cursorFrames.count - 1];
		cursorFrames.PopBack();
	}
	
	void PushVarFrame(){
		varFrames.PushBack(varsInScope.count);
	}
	
	void PopVarFrame(){
		int oldSize = varFrames.data[varFrames.count - 1];
		varFrames.PopBack();
		
		while(varsInScope.count > oldSize){
			varsInScope.PopBack();
		}
	}

	FuncDef* GetFuncDef(const SubString& name) const;
	TypeInfo* GetVariableType(const SubString& name) const;
	int GetVariableOffset(const SubString& name) const;
	int GetStackFrameOffset() const;

	bool ShuntingYard(const Vector<BNVToken>& inToks, Vector<BNVToken>& outToks);
	
	Value* ParseValue();
	Scope* ParseScope();
	
	TypeInfo* ParseType();
	TypeInfo* ParseVarName();
	bool ParseIdentifier(SubString* outStr);
	
	//Does not include ";" or ","
	bool ExpectAndEatVarDecl(VarDecl* out){
		PushCursorFrame();
		
		if(TypeInfo* info = ParseType()){
			SubString str;
			if(ParseIdentifier(&str)){
				out->name = str;
				out->type = info;
				return true;
			}
			else{
				PopCursorFrame();
				return false;
			}
		}
		else{
			PopCursorFrame();
			return false;
		}
	}
};


#endif

