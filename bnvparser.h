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

#define BNS_SAFE_DELETE(x) delete (x); (x) = nullptr;

struct Statement{
	virtual void AddByteCode(BNVM& vm) = 0;
	virtual TypeInfo* TypeCheck(const BNVParser& parser) = 0;

	virtual ~Statement(){}
};

struct VariableDeclaration : Statement{
	virtual void AddByteCode(BNVM& vm) {

	}
	virtual TypeInfo* TypeCheck(const BNVParser& parser) {
		return nullptr;
	}

	virtual ~VariableDeclaration(){}
};

struct ReturnStatement : Statement{
	Value* retVal;
	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);

	virtual ~ReturnStatement();
};

struct Scope : Statement{
	Vector<Statement*> statements;
	
	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);

	virtual ~Scope(){
		for(int i = 0; i < statements.count; i++){
			BNS_SAFE_DELETE(statements.data[i]);
		}
	}
};

struct IfStatement : Scope{
	Value* check;
	
	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);

	virtual ~IfStatement();
};

struct WhileStatement : IfStatement{
	virtual void AddByteCode(BNVM& vm);

	virtual ~WhileStatement(){}
};


struct Value : Statement{
	TypeInfo* type;
	virtual void AddByteCode(BNVM& vm) = 0;
	virtual TypeInfo* TypeCheck(const BNVParser& parser) = 0;

	virtual ~Value(){}
};

struct IntLiteral : Value {
	int value;

	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);

	virtual ~IntLiteral(){}
};

struct FloatLiteral : Value {
	float value;

	FloatLiteral() {
		value = -12;
	}
	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);

	virtual ~FloatLiteral(){}
};

struct VoidLiteral : Value {
	virtual void AddByteCode(BNVM& vm) {}
	virtual TypeInfo* TypeCheck(const BNVParser& parser);
	virtual ~VoidLiteral(){}
};

struct BinaryOp : Value{
	Value* lVal;
	Value* rVal;
	SubString op;
	
	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);

	virtual ~BinaryOp(){BNS_SAFE_DELETE(lVal); BNS_SAFE_DELETE(rVal);}
};

struct UnaryOp : Value{
	Value* val;
	SubString op;
	
	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);

	virtual ~UnaryOp(){BNS_SAFE_DELETE(val);}
};

struct VariableAccess;

struct Assignment : Statement{
	VariableAccess* var;
	Value* val;
	
	virtual TypeInfo* TypeCheck(const BNVParser& parser);
	virtual void AddByteCode(BNVM& vm);
	virtual ~Assignment();
};

struct FunctionCall : Value{
	FuncDef* func;
	int stackFrameOffset;

	Vector<Value*> args;

	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);

	virtual ~FunctionCall(){for(int i = 0; i < args.count; i++){BNS_SAFE_DELETE(args.data[i]);}}
};

struct VariableAccess : Value {
	SubString varName;
	int regOffset;

	VariableAccess() {
		regOffset = -1;
	}

	virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);

	virtual ~VariableAccess(){}
};

struct FieldAccess : VariableAccess {
	VariableAccess* var;
	SubString fieldName;

	//virtual void AddByteCode(BNVM& vm);
	virtual TypeInfo* TypeCheck(const BNVParser& parser);

	virtual ~FieldAccess(){BNS_SAFE_DELETE(var);}
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

	virtual ~FuncDef(){}
};

struct ExternFuncDef : FuncDef {
	virtual void AddByteCode(BNVM& vm) override{}
	virtual TypeInfo* TypeCheck(const BNVParser& parser) override {
		return nullptr; 
	}
	virtual ~ExternFuncDef() {
		int xxg = 0;
	}
};

struct TypeInfo {
	SubString typeName;
	int size;
};

struct StructDef{
	SubString name;
	Vector<VarDecl> fields;

	int GetSize() const {
		int size = 0;
		for (int i = 0; i < fields.count; i++) {
			size += fields.data[i].type->size;
		}

		return size;
	}
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

	StringMap<int> externFuncNames;

	BNVParser();
	
	~BNVParser(){
		for(int i = 0; i < structDefs.count; i++){
			BNS_SAFE_DELETE(structDefs.data[i]);
		}
		for(int i = 0; i < funcDefs.count; i++){
			BNS_SAFE_DELETE(funcDefs.data[i]);
		}
		for(int i = 0; i < definedTypes.count; i++){
			BNS_SAFE_DELETE(definedTypes.values[i]);
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
	StructDef* GetStructDef(const SubString& name) const;
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

