#include "lexer.h"

#include <stdio.h>
#include <time.h>

#include "macros.h"

typedef enum {
	LS_STRING,
	LS_CHARACTER,
	LS_INTEGER,
	LS_FLOAT,
	LS_OCTAL,
	LS_HEX,
	LS_OPERATOR,
	LS_IDENTIFIER,
	LS_LINECOMMENT,
	LS_ANNOTATION_STR,
	LS_ANNOTATION_ID,
	LS_ANNOTATION_WS,
	LS_BLOCKCOMMENT,
	LS_WHITESPACE
} LexerState;

static const char* whitespace = "\t\r\n ";

static const char* operators[] = { ",", "++", "--", "*", "/", "->", "+", "-", "&", "|", "&&", "||", "#", "%", "{", "}", ";", "(", ")"
"~", "^", "!=", "==", "=", "!", ".", "..", "?", ":", "<", ">", "<<", ">>", "<=", ">=", "[", "]", "+=",
"-=", "*=", "/=", "^=", "|=", "&=", "##", "::" };

int GetOperatorCount() {
	return BNS_ARRAY_COUNT(operators);
}

const char** GetOperators() {
	return operators;
}

Vector<SubString> LexString(String string) {

	Vector<SubString> tokens;
	LexerState currState = LS_WHITESPACE;

#define EMIT_TOKEN() {currToken.length = (int)(fileCursor - currToken.start+1);tokens.PushBack(currToken);\
						  currToken.length = 0; currToken.start = fileCursor+1;}

	int fileSize = string.GetLength();

	char* fileCursor = string.string;
	SubString currToken = string.GetSubString(0, 0);

	while (fileCursor - string.string < fileSize) {
		switch (currState) {
		case LS_STRING: {
			if (*fileCursor == '\\') {
				fileCursor++;
			}
			else if (*fileCursor == '"') {
				EMIT_TOKEN();
				currState = LS_WHITESPACE;
			}
		}break;

		case LS_CHARACTER: {
			if (*fileCursor == '\\') {
				fileCursor++;
			}
			else if (*fileCursor == '\'') {
				EMIT_TOKEN();
				currState = LS_WHITESPACE;
			}
		}break;

		case LS_INTEGER: {
			if (*fileCursor == '.') {
				currState = LS_FLOAT;
			}
			else if (*fileCursor < '0' || *fileCursor > '9') {
				fileCursor--;
				EMIT_TOKEN();
				currState = LS_WHITESPACE;
			}
		}break;

		case LS_FLOAT: {
			if (*fileCursor < '0' || *fileCursor > '9') {
				if (*fileCursor != 'f') {
					fileCursor--;
				}

				EMIT_TOKEN();
				currState = LS_WHITESPACE;
			}
		}break;

		case LS_OCTAL: {
			if (*fileCursor == 'x' || *fileCursor == 'X') {
				currState = LS_HEX;
			}
			else if (*fileCursor == '.') {
				currState = LS_FLOAT;
			}
			else if (*fileCursor < '0' || *fileCursor > '9') {
				fileCursor--;
				EMIT_TOKEN();
				currState = LS_WHITESPACE;
			}
		}break;

		case LS_HEX: {
			if ((*fileCursor < '0' || *fileCursor > '9') && (*fileCursor < 'a' || *fileCursor > 'f')
				&& (*fileCursor < 'A' || *fileCursor > 'F')) {
				fileCursor--;
				EMIT_TOKEN();
				currState = LS_WHITESPACE;
			}
		}break;

		case LS_OPERATOR: {
			bool found = false;

			for (int i = 0; i < BNS_ARRAY_COUNT(operators); i++) {
				int strLen = StrLen(operators[i]);
				int tokLen = fileCursor - currToken.start + 1;
				if (tokLen <= strLen && memcmp(currToken.start, operators[i], tokLen) == 0) {
					found = true;
					break;
				}
			}

			if (!found) {
				fileCursor--;
				EMIT_TOKEN();
				currState = LS_WHITESPACE;
			}
		}break;

		case LS_IDENTIFIER: {
			if (FindChar(whitespace, *fileCursor) != -1) {
				fileCursor--;
				EMIT_TOKEN();
				currState = LS_WHITESPACE;
			}
			else {
				bool isOp = false;
				for (int i = 0; i < BNS_ARRAY_COUNT(operators); i++) {
					if (operators[i][0] == *fileCursor) {
						isOp = true;
						break;
					}
				}

				if (isOp) {
					fileCursor--;
					EMIT_TOKEN();
					currState = LS_WHITESPACE;
				}
			}
		}break;

		case LS_LINECOMMENT: {
			if (*fileCursor == '\n') {
				currState = LS_WHITESPACE;
			}

			currToken.start++;
		}break;

		case LS_ANNOTATION_STR: {
			if (*fileCursor == '"') {
				EMIT_TOKEN();
				break;
			}
		}
		case LS_ANNOTATION_ID: {
			if (*fileCursor == ']') {
				fileCursor--;
				EMIT_TOKEN();
				tokens.PushBack(string.GetSubString((int)(fileCursor - string.string + 1), 3));
				currState = LS_BLOCKCOMMENT;
			}
			else if (FindChar(whitespace, *fileCursor) != -1 || *fileCursor == '(') {
				fileCursor--;
				EMIT_TOKEN();
				currState = LS_ANNOTATION_WS;
			}
		}break;

		case LS_ANNOTATION_WS: {
			if (*fileCursor == ']') {
				tokens.PushBack(string.GetSubString((int)(fileCursor - string.string), 3));
				currState = LS_BLOCKCOMMENT;
				break;
			}
			else if (*fileCursor == '(' || *fileCursor == ')') {
				SubString thisTok = string.GetSubString((int)(fileCursor - string.string), 1);
				tokens.PushBack(thisTok);
				break;
			}
			else if (*fileCursor == '"') {
				currToken.start = fileCursor - 1;
				currState = LS_ANNOTATION_STR;
			}
			else if (FindChar(whitespace, *fileCursor) == -1) {
				currState = LS_ANNOTATION_ID;
			}
		}

		case LS_BLOCKCOMMENT: {
			if (fileCursor - string.string < fileSize - 2 && fileCursor[0] == '*' && fileCursor[1] == '/') {
				currState = LS_WHITESPACE;
				fileCursor += 1;
				currToken.start += 1;
			}
			else if (*fileCursor == '[' && *(fileCursor - 1) == '*' && *(fileCursor - 2) == '/') {
				tokens.PushBack(string.GetSubString((int)(fileCursor - string.string - 2), 3));
				currState = LS_ANNOTATION_WS;
			}

			currToken.start++;
		}break;

		case LS_WHITESPACE: {
			currToken.start = fileCursor;
			if (fileCursor - string.string < fileSize - 2 && fileCursor[0] == '/' && fileCursor[1] == '/') {
				currState = LS_LINECOMMENT;
			}
			else if (fileCursor - string.string < fileSize - 2 && fileCursor[0] == '/' && fileCursor[1] == '*') {
				currState = LS_BLOCKCOMMENT;
			}
			else if (FindChar(whitespace, *fileCursor) == -1) {

				if (*fileCursor == '"') {
					currState = LS_STRING;
					fileCursor++;
				}
				else if (*fileCursor == '\'') {
					currState = LS_CHARACTER;
					fileCursor++;
				}
				else if (*fileCursor == '0') {
					currState = LS_OCTAL;
				}
				else if (*fileCursor >= '1' && *fileCursor <= '9') {
					currState = LS_INTEGER;
				}
				else {
					bool isOp = false;
					for (int i = 0; i < BNS_ARRAY_COUNT(operators); i++) {
						if (operators[i][0] == *fileCursor) {
							isOp = true;
							break;
						}
					}

					if (isOp) {
						currState = LS_OPERATOR;
					}
					else {
						currState = LS_IDENTIFIER;
					}
				}

				fileCursor--;
			}
		}break;
		}

		fileCursor++;
	}

	if (currState != LS_WHITESPACE && currState != LS_BLOCKCOMMENT && currState != LS_LINECOMMENT) {
		fileCursor--;
		EMIT_TOKEN();
		fileCursor++;
	}

#undef EMIT_TOKEN

	return tokens;
}

#if defined(LEXER_TEST_MAIN)

int main(int argc, char** argv) {

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
		const char* expectedToks[] = { "if", "(", "this", ".",
			"Marvel", "(", ")", "==",
			"that", "->", "goal", "(",
			")", ")", "{", "iter",
			"++", ";", "}" };

		Vector<SubString> actualToks = LexString(string);

		ASSERT(BNS_ARRAY_COUNT(expectedToks) == actualToks.count);

		for (int i = 0; i < actualToks.count; i++) {
			ASSERT(actualToks.data[i] == expectedToks[i]);
		}
	}
	
	{
		const char* string = "1.234+2.345";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 3);
		ASSERT(actualToks.data[0] == "1.234");
		ASSERT(actualToks.data[1] == "+");
		ASSERT(actualToks.data[2] == "2.345");
	}

	{
		const char* string = "1.234*2.345+4";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 5);
		ASSERT(actualToks.data[0] == "1.234");
		ASSERT(actualToks.data[1] == "*");
		ASSERT(actualToks.data[2] == "2.345");
		ASSERT(actualToks.data[3] == "+");
		ASSERT(actualToks.data[4] == "4");
	}
	
	{
		const char* string = "/*24*/5";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 1);
		ASSERT(actualToks.data[0] == "5");
	}
	
	{
		const char* string = "/*24*/5//45\n6";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 2);
		ASSERT(actualToks.data[0] == "5");
		ASSERT(actualToks.data[1] == "6");
	}
	
	{
		const char* string = "/*[Arr]*///45\n6";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 4);
		ASSERT(actualToks.data[0] == "/*[");
		ASSERT(actualToks.data[1] == "Arr");
		ASSERT(actualToks.data[2] == "]*/");
		ASSERT(actualToks.data[3] == "6");
	}

	{
		const char* string = "/*[Arr]*/56//45\n6";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 5);
		ASSERT(actualToks.data[0] == "/*[");
		ASSERT(actualToks.data[1] == "Arr");
		ASSERT(actualToks.data[2] == "]*/");
		ASSERT(actualToks.data[3] == "56");
		ASSERT(actualToks.data[4] == "6");
	}

	{
		const char* string = "/*[Arr()]*/56//45\n6";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 7);
		ASSERT(actualToks.data[0] == "/*[");
		ASSERT(actualToks.data[1] == "Arr");
		ASSERT(actualToks.data[2] == "(");
		ASSERT(actualToks.data[3] == ")");
		ASSERT(actualToks.data[4] == "]*/");
		ASSERT(actualToks.data[5] == "56");
		ASSERT(actualToks.data[6] == "6");
	}

	{
		const char* string = "/*[Arr(\"gge\")]*/56//45\n6";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 8);
		ASSERT(actualToks.data[0] == "/*[");
		ASSERT(actualToks.data[1] == "Arr");
		ASSERT(actualToks.data[2] == "(");
		ASSERT(actualToks.data[3] == "\"gge\"");
		ASSERT(actualToks.data[4] == ")");
		ASSERT(actualToks.data[5] == "]*/");
		ASSERT(actualToks.data[6] == "56");
		ASSERT(actualToks.data[7] == "6");
	}

	{
		const char* string = "23/*24*/5";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 2);
		ASSERT(actualToks.data[0] == "23");
		ASSERT(actualToks.data[1] == "5");
	}

	{
		const char* string = "023/*24*/5";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 2);
		ASSERT(actualToks.data[0] == "023");
		ASSERT(actualToks.data[1] == "5");
	}

	{
		const char* string = "0x23/*24*/5";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 2);
		ASSERT(actualToks.data[0] == "0x23");
		ASSERT(actualToks.data[1] == "5");
	}

	{
		const char* string = "0X23/*24*/5";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 2);
		ASSERT(actualToks.data[0] == "0X23");
		ASSERT(actualToks.data[1] == "5");
	}

	{
		const char* string = "0X2DEADBEEF3/*24*/5";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 2);
		ASSERT(actualToks.data[0] == "0X2DEADBEEF3");
		ASSERT(actualToks.data[1] == "5");
	}

	{
		const char* string = "0X2deadbeef3/*24*/5";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 2);
		ASSERT(actualToks.data[0] == "0X2deadbeef3");
		ASSERT(actualToks.data[1] == "5");
	}

	{
		const char* string = "23\"er\"45";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 3);
		ASSERT(actualToks.data[0] == "23");
		ASSERT(actualToks.data[1] == "\"er\"");
		ASSERT(actualToks.data[2] == "45");
	}

	{
		const char* string = "23\"er\"//45";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 2);
		ASSERT(actualToks.data[0] == "23");
		ASSERT(actualToks.data[1] == "\"er\"");
	}

	{
		const char* string = "23\"er\"//45\n34";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 3);
		ASSERT(actualToks.data[0] == "23");
		ASSERT(actualToks.data[1] == "\"er\"");
		ASSERT(actualToks.data[2] == "34");
	}

	{
		const char* string = "23\"er\\\"\"//45";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 2);
		ASSERT(actualToks.data[0] == "23");
		ASSERT(actualToks.data[1] == "\"er\\\"\"");
	}

	{
		const char* string = "'e'";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 1);
		ASSERT(actualToks.data[0] == "'e'");
	}

	{
		const char* string = "'\\''";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 1);
		ASSERT(actualToks.data[0] == "'\\''");
	}

	{
		const char* string = "'A'5//er\n4";
		Vector<SubString> actualToks = LexString(string);
		ASSERT(actualToks.count == 3);
		ASSERT(actualToks.data[0] == "'A'");
		ASSERT(actualToks.data[1] == "5");
		ASSERT(actualToks.data[2] == "4");
	}

	return 0;
}

#endif
