#include "lexer.h"

#include <stdio.h>
#include <time.h>

#include "macros.h"

typedef enum{
	STRING,
	CHARACTER,
	INTEGER,
	FLOAT,
	OCTAL,
	HEX,
	OPERATOR,
	IDENTIFIER,
	LINECOMMENT,
	ANNOTATION_STR,
	ANNOTATION_ID,
	ANNOTATION_WS,
	BLOCKCOMMENT,
	WHITESPACE
} LexerState;

static const char* whitespace = "\t\r\n ";

static const char* operators[] = {  ",", "++", "--", "*", "/", "->", "+", "-", "&", "|", "&&", "||", "#", "%", "{", "}", ";", "(", ")"
									"~", "^", "!=", "==", "=", "!", ".", "?", ":", "<", ">", "<<", ">>", "<=", ">=", "[", "]", "+=",
									"-=", "*=", "/=", "^=", "|=", "&=", "##"};
							
int GetOperatorCount(){
	return BNS_ARRAY_COUNT(operators);
}

const char** GetOperators(){
	return operators;
}

Vector<SubString> LexString(String string){
	
	String annoStart = "/*[";
	String annoEnd = "]*/";
	
	Vector<SubString> tokens;	
	LexerState currState = WHITESPACE;
	
	#define EMIT_TOKEN() {currToken.length = (int)(fileCursor - currToken.start+1);tokens.PushBack(currToken);\
						  currToken.length = 0; currToken.start = fileCursor+1;}
	
	int fileSize = string.GetLength();
	
	char* fileCursor = string.string;
	SubString currToken = string.GetSubString(0, 0);

	while(fileCursor - string.string < fileSize){
		switch(currState){
			case STRING:{
				if(*fileCursor == '\\'){
					fileCursor++;
				}
				else if(*fileCursor == '"'){
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case CHARACTER:{
				if(*fileCursor == '\\'){
					fileCursor++;
				}
				else if(*fileCursor == '\''){
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case INTEGER:{
				if(*fileCursor == '.'){
					currState = FLOAT;
				}
				else if(*fileCursor < '0' || *fileCursor > '9'){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case FLOAT:{
				if(*fileCursor < '0' || *fileCursor > '9'){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case OCTAL:{
				if(*fileCursor < '0' || *fileCursor > '9'){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
				else if(*fileCursor == 'x' || *fileCursor == 'X'){
					currState = HEX;
				}
			}break;
			
			case HEX:{
				if((*fileCursor < '0' || *fileCursor > '9') && (*fileCursor < 'a' || *fileCursor > 'f')
					&& (*fileCursor < 'A' || *fileCursor > 'F')){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case OPERATOR:{
				bool found = false;

				for(int i = 0; i < BNS_ARRAY_COUNT(operators); i++){
					if(memcmp(currToken.start, operators[i], fileCursor - currToken.start + 1) == 0){
						found = true;
						break;
					}
				}
				
				if(!found){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case IDENTIFIER:{		
				if(FindChar(whitespace, *fileCursor) != -1){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
				else{
					bool isOp = false;
					for(int  i = 0; i < BNS_ARRAY_COUNT(operators); i++){
						if(operators[i][0] == *fileCursor){
							isOp = true;
							break;
						}
					}
					
					if(isOp){
						fileCursor--;
						EMIT_TOKEN();
						currState = WHITESPACE;
					}
				}
			}break;
			
			case LINECOMMENT:{
				if(*fileCursor == '\n'){
					currState = WHITESPACE;
				}
				
				currToken.start++;
			}break;
			
			case ANNOTATION_STR:{
				if(*fileCursor == '"'){
					EMIT_TOKEN();
					break;
				}
			}
			case ANNOTATION_ID:{
				if(*fileCursor == ']'){
					fileCursor--;
					EMIT_TOKEN();
					tokens.PushBack(annoEnd.GetSubString(0,3));
					currState = BLOCKCOMMENT;
				}
				else if(FindChar(whitespace, *fileCursor) != -1 || *fileCursor == '('){
					fileCursor--;
					EMIT_TOKEN();
					currState = ANNOTATION_WS;
				}
			}break;

			case ANNOTATION_WS:{
				if(*fileCursor == ']'){
					tokens.PushBack(annoEnd.GetSubString(0,3));
					currState = BLOCKCOMMENT;
					break;
				}
				else if(*fileCursor == '(' || *fileCursor == ')'){
					SubString thisTok = string.GetSubString(fileCursor - string.string, 1);
					tokens.PushBack(thisTok);
					break;
				}
				else if(*fileCursor == '"'){
					currState = ANNOTATION_STR;
				}
				else if(FindChar(whitespace, *fileCursor) == -1){
					currState = ANNOTATION_ID;
				}
			}
			
			case BLOCKCOMMENT:{
				if(fileCursor - string.string < fileSize - 2 && fileCursor[0] == '*' && fileCursor[1] == '/'){
					currState = WHITESPACE;
					fileCursor += 2;
					currToken.start += 2;
				}
				else if(*fileCursor == '[' && *(fileCursor - 1) == '*' && *(fileCursor - 2) == '/'){
					tokens.PushBack(annoStart.GetSubString(0,3));
					currState = ANNOTATION_WS;
				}
				
				currToken.start++;
			}break;
			
			case WHITESPACE:{
				currToken.start = fileCursor;
				if(fileCursor - string.string < fileSize - 2 && fileCursor[0] == '/' && fileCursor[1] == '/'){
					currState = LINECOMMENT;
				}
				else if(fileCursor - string.string < fileSize - 2 && fileCursor[0] == '/' && fileCursor[1] == '*'){
					currState = BLOCKCOMMENT;
				}
				else if(FindChar(whitespace, *fileCursor) == -1){
					
					if(*fileCursor == '"'){
						currState = STRING;
						fileCursor++;
					}
					else if(*fileCursor == '\''){
						currState = CHARACTER;
						fileCursor++;
					}
					else if(*fileCursor == '0'){
						currState = OCTAL;
					}
					else if(*fileCursor >= '1' && *fileCursor <= '9'){
						currState = INTEGER;
					}
					else{
						bool isOp = false;
						for(int i = 0; i < BNS_ARRAY_COUNT(operators); i++){
							if(operators[i][0] == *fileCursor){
								isOp = true;
								break;
							}
						}
						
						if(isOp){
							currState = OPERATOR;
						}
						else{
							currState = IDENTIFIER;
						}
					}
					
					fileCursor--;
				}
			}break;
		}
		
		fileCursor++;
	}
	
	if(currState != WHITESPACE){
		fileCursor--;
		EMIT_TOKEN();
		fileCursor++;
	}
	
	#undef EMIT_TOKEN
	
	return tokens;
}

#if defined(LEXER_TEST_MAIN)

int main(int argc, char** argv){
	
	BNS_UNUSED(argc);
	BNS_UNUSED(argv);
	
	{
		ASSERT(FindChar("abbbbc", 'c') == 5);
		ASSERT(FindChar("abbbbc", 'a') == 0);
		ASSERT(FindChar("abbbbc", 'd') == -1);
		ASSERT(FindChar("abbbbc", 'A') == -1);
		ASSERT(FindChar("  \t", '\t') == 2);
	}
	
	{
		Vector<SubString> lexedToks = LexString("1+2*3");
		
		ASSERT(lexedToks.count == 5);
		ASSERT(lexedToks.data[0] == "1");
		ASSERT(lexedToks.data[1] == "+");
		ASSERT(lexedToks.data[2] == "2");
	}
	
	{
		Vector<SubString> lexedToks = LexString("if (x == 2)  \t\t\t\n\n\n\n334");
		
		ASSERT(lexedToks.count == 7);
		
		ASSERT(lexedToks.data[6] == "334");
	}
	
	{
		const char* string = "if(this.Marvel() == that->goal()){iter++;}";
		const char* expectedToks[] = {"if", "(", "this", ".",
									 "Marvel", "(", ")", "==", 
									 "that", "->", "goal", "(", 
									 ")", ")", "{", "iter", 
									 "++", ";", "}"}; 
								   
		Vector<SubString> actualToks = LexString(string);
		
		ASSERT(BNS_ARRAY_COUNT(expectedToks) == actualToks.count);
		
		for(int i = 0; i < actualToks.count; i++){
			ASSERT(actualToks.data[i] == expectedToks[i]);
		}
	}

	return 0;
}

#endif
