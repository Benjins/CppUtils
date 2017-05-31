#include "sexpr.h"

enum SexprLexerState {
	LS_DoubleQuote,
	LS_SingleQuote,
	LS_WhiteSpace,
	LS_Other
};

inline bool IsWhiteSpace(char c) {
	return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
}

inline bool IsDigit(char c) {
	return (c >= '0') && (c <= '9');
}

Vector<SubString> BNSexprLexTokens(const String& toParse) {
	const char* cursor = toParse.string;
	const char* tokStart = cursor;
	SexprLexerState state = LS_WhiteSpace;
	Vector<SubString> tokens;
#define EMIT_TOKEN() do { \
	int start = tokStart - toParse.string; \
	int end = cursor - toParse.string; \
	if (start != end) { \
		tokens.PushBack(toParse.GetSubString(start, end - start)); \
	} \
	tokStart = cursor;\
	} while(0)

	while (cursor && *cursor) {
		switch (state) {
		case LS_DoubleQuote: {
			if (*cursor == '"') { cursor++; EMIT_TOKEN(); cursor--;  state = LS_WhiteSpace; }
			else if (*cursor == '\\') { cursor++; }
		} break;
		case LS_SingleQuote: {
			if (*cursor == '\'') { cursor++; EMIT_TOKEN(); cursor--;  state = LS_WhiteSpace; }
			else if (*cursor == '\\') { cursor++; }
		} break;
		case LS_WhiteSpace: {
			tokStart = cursor;

			if (*cursor == '"') { state = LS_DoubleQuote; }
			else if (*cursor == '\'') { state = LS_SingleQuote; }
			else if (*cursor == '(' || *cursor == ')') {
				cursor++;
				EMIT_TOKEN();
				cursor--;
				state = LS_WhiteSpace;
			}
			else if (!IsWhiteSpace(*cursor)) { state = LS_Other; }
		} break;
		case LS_Other: {
			if (*cursor == '"') { EMIT_TOKEN();  state = LS_DoubleQuote; }
			else if (*cursor == '\'') { EMIT_TOKEN(); state = LS_SingleQuote; }
			else if (IsWhiteSpace(*cursor)) { EMIT_TOKEN(); state = LS_WhiteSpace; }
			else if (*cursor == '(' || *cursor == ')') {
				EMIT_TOKEN();
				cursor++;
				EMIT_TOKEN();
				cursor--;
				state = LS_WhiteSpace;
			}
		} break;
		}

		cursor++;
	}

	EMIT_TOKEN();

#undef EMIT_TOKEN

	return tokens;
}

bool ParseSexpr(BNSexpr* outSexpr, const Vector<SubString>& toks, int* index);

bool ParseSexprParenList(BNSexpr* outSexpr, const Vector<SubString>& tokens, int* index) {
	int oldIndex = *index;
	if (tokens.data[*index] == "(") {
		(*index)++;
		Vector<BNSexpr> children;
		while (true) {
			BNSexpr sexpr;
			if (tokens.data[*index] == ")") {
				(*index)++;
				BNSexprParenList parent;
				// TODO: avoid copy
				parent.children = children;
				*outSexpr = parent;
				return true;
			}
			else if (ParseSexpr(&sexpr, tokens, index)) {
				children.PushBack(sexpr);
			}
			else {
				break;
			}
		}
	}

	*index = oldIndex;
	return false;
}

bool ParseSexprString(BNSexpr* outSexpr, const Vector<SubString>& tokens, int* index) {
	if (tokens.data[*index].start[0] == '"' || tokens.data[*index].start[0] == '\'') {
		BNSexprString str;
		str.value = tokens.data[*index];
		*outSexpr = str;
		(*index)++;
		return true;
	}

	return false;
}

bool ParseSexprIdentifer(BNSexpr* outSexpr, const Vector<SubString>& tokens, int* index) {
	if (tokens.data[*index].start[0] != '"' && tokens.data[*index].start[0] != '\'') {
		BNSexprIdentifier ident;
		ident.identifier = tokens.data[*index];
		*outSexpr = ident;
		(*index)++;
		return true;
	}

	return false;
}

bool ParseSexprNum(BNSexpr* outSexpr, const Vector<SubString>& tokens, int* index) {
	const SubString& tok = tokens.data[*index];

	bool isNum = true;
	bool isFloat = false;
	int i = 0;
	if (tok.start[i] == '-' || tok.start[i] == '.') {
		if (tok.length == 1) { return false; }
		else { i++; }
	}

	for (;  i < tok.length; i++) {
		if (!IsDigit(tok.start[i]) && tok.start[i] != '.') {
			isNum = false;
			break;
		}
		else if (tok.start[i] == '.') {
			isFloat = true;
		}
	}

	if (isNum) {
		(*index)++;
		if (isFloat) {
			BNSexprNumber num = BNSexprNumber(Atof(tok.start));
			*outSexpr = num;
		}
		else {
			BNSexprNumber num = BNSexprNumber((long long)Atoi(tok.start));
			*outSexpr = num;
		}
		return true;
	}
	else {
		return false;
	}
}

bool ParseSexpr(BNSexpr* outSexpr, const Vector<SubString>& tokens, int* index) {
	if (ParseSexprParenList(outSexpr, tokens, index)) {
		return true;
	}
	else if (ParseSexprString(outSexpr, tokens, index)) {
		return true;
	}
	else if (ParseSexprNum(outSexpr, tokens, index)) {
		return true;
	}
	else if (ParseSexprIdentifer(outSexpr, tokens, index)) {
		return true;
	}
	else {
		return false;
	}
}

BNSexprParseResult ParseSexprs(Vector<BNSexpr>* outSexprs, const String& str) {
	Vector<SubString> tokens = BNSexprLexTokens(str);
	int i = 0;
	while (i < tokens.count) {
		BNSexpr sexpr;
		if (ParseSexpr(&sexpr, tokens, &i)) {
			outSexprs->PushBack(sexpr);
		}
		else {
			return BNSexpr_Error;
		}
	}

	return BNSexpr_Success;
}

#if defined(BNSEXPR_TEST_MAIN)

int main(int argc, char** argv) {

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(this is me)");

		ASSERT(sexprs.count == 1);
		ASSERT(sexprs.data[0].type == BNSexpr::UE_BNSexprParenList);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.count == 3);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[0].type == BNSexpr::UE_BNSexprIdentifier);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[1].type == BNSexpr::UE_BNSexprIdentifier);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[2].type == BNSexpr::UE_BNSexprIdentifier);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[0].AsBNSexprIdentifier().identifier == "this");
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[1].AsBNSexprIdentifier().identifier == "is");
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[2].AsBNSexprIdentifier().identifier == "me");
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "()");

		ASSERT(sexprs.count == 1);
		ASSERT(sexprs.data[0].type == BNSexpr::UE_BNSexprParenList);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.count == 0);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(this)");

		ASSERT(sexprs.count == 1);
		ASSERT(sexprs.data[0].type == BNSexpr::UE_BNSexprParenList);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.count == 1);

		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[0].type == BNSexpr::UE_BNSexprIdentifier);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[0].AsBNSexprIdentifier().identifier == "this");
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "\"hello\"");

		ASSERT(sexprs.count == 1);
		ASSERT(sexprs.data[0].type == BNSexpr::UE_BNSexprString);
		ASSERT(sexprs.data[0].AsBNSexprString().value == "\"hello\"");
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "'hello'");

		ASSERT(sexprs.count == 1);
		ASSERT(sexprs.data[0].type == BNSexpr::UE_BNSexprString);
		ASSERT(sexprs.data[0].AsBNSexprString().value == "'hello'");
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(yo 'hello')");

		ASSERT(sexprs.count == 1);
		ASSERT(sexprs.data[0].type == BNSexpr::UE_BNSexprParenList);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.count == 2);

		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[0].type == BNSexpr::UE_BNSexprIdentifier);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[0].AsBNSexprIdentifier().identifier == "yo");

		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[1].type == BNSexpr::UE_BNSexprString);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[1].AsBNSexprString().value == "'hello'");
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(yo 'hello' (sup 'yes'))");

		ASSERT(sexprs.count == 1);
		ASSERT(sexprs.data[0].type == BNSexpr::UE_BNSexprParenList);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.count == 3);

		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[0].type == BNSexpr::UE_BNSexprIdentifier);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[0].AsBNSexprIdentifier().identifier == "yo");

		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[1].type == BNSexpr::UE_BNSexprString);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[1].AsBNSexprString().value == "'hello'");

		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[2].type == BNSexpr::UE_BNSexprParenList);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[2].AsBNSexprParenList().children.count == 2);

		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[2].AsBNSexprParenList().children.data[0].type
			== BNSexpr::UE_BNSexprIdentifier);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[2].AsBNSexprParenList().children.data[0].AsBNSexprIdentifier().identifier
			== "sup");
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[2].AsBNSexprParenList().children.data[1].type
			== BNSexpr::UE_BNSexprString);
		ASSERT(sexprs.data[0].AsBNSexprParenList().children.data[2].AsBNSexprParenList().children.data[1].AsBNSexprString().value
			== "'yes'");
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "2334");

		ASSERT(sexprs.count == 1);
		ASSERT(sexprs.data[0].type == BNSexpr::UE_BNSexprNumber);
		ASSERT(sexprs.data[0].AsBNSexprNumber().isFloat == false);
		ASSERT(sexprs.data[0].AsBNSexprNumber().iValue == 2334);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "23 34");

		ASSERT(sexprs.count == 2);
		ASSERT(sexprs.data[0].type == BNSexpr::UE_BNSexprNumber);
		ASSERT(sexprs.data[0].AsBNSexprNumber().isFloat == false);
		ASSERT(sexprs.data[0].AsBNSexprNumber().iValue == 23);
		ASSERT(sexprs.data[1].type == BNSexpr::UE_BNSexprNumber);
		ASSERT(sexprs.data[1].AsBNSexprNumber().isFloat == false);
		ASSERT(sexprs.data[1].AsBNSexprNumber().iValue == 34);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "23.125");

		ASSERT(sexprs.count == 1);
		ASSERT(sexprs.data[0].type == BNSexpr::UE_BNSexprNumber);
		ASSERT(sexprs.data[0].AsBNSexprNumber().isFloat == true);
		ASSERT(sexprs.data[0].AsBNSexprNumber().fValue == 23.125);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "-");

		ASSERT(sexprs.count == 1);
		ASSERT(sexprs.data[0].type == BNSexpr::UE_BNSexprIdentifier);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, ".");

		ASSERT(sexprs.count == 1);
		ASSERT(sexprs.data[0].type == BNSexpr::UE_BNSexprIdentifier);
	}

	{
		Vector<SubString> toks = BNSexprLexTokens("((()))");
		ASSERT(toks.count == 6);
		for (int i = 0; i < 3; i++) { ASSERT(toks.data[i] == "("); }
		for (int i = 3; i < 6; i++) { ASSERT(toks.data[i] == ")"); }
	}

	{
		Vector<SubString> toks = BNSexprLexTokens("(this is (1.232)\"meeee\")");

		ASSERT(toks.count == 8);
		ASSERT(toks.data[0] == "(");
		ASSERT(toks.data[1] == "this");
		ASSERT(toks.data[2] == "is");
		ASSERT(toks.data[3] == "(");
		ASSERT(toks.data[4] == "1.232");
		ASSERT(toks.data[5] == ")");
		ASSERT(toks.data[6] == "\"meeee\"");
		ASSERT(toks.data[7] == ")");
	}

	{
		Vector<SubString> toks = BNSexprLexTokens("()");

		ASSERT(toks.count == 2);
		ASSERT(toks.data[0] == "(");
		ASSERT(toks.data[1] == ")");
	}

	{
		Vector<SubString> toks = BNSexprLexTokens("helllo");

		ASSERT(toks.count == 1);
		ASSERT(toks.data[0] == "helllo");
	}

	{
		Vector<SubString> toks = BNSexprLexTokens("\"\"\"\"");

		ASSERT(toks.count == 2);
		ASSERT(toks.data[0] == "\"\"");
		ASSERT(toks.data[1] == "\"\"");
	}

	{
		Vector<SubString> toks = BNSexprLexTokens("\"aa\"\"bb\"");

		ASSERT(toks.count == 2);
		ASSERT(toks.data[0] == "\"aa\"");
		ASSERT(toks.data[1] == "\"bb\"");
	}

	{
		Vector<SubString> toks = BNSexprLexTokens("\"aa\"(1.2\"\")");

		ASSERT(toks.count == 5);
		ASSERT(toks.data[0] == "\"aa\"");
		ASSERT(toks.data[1] == "(");
		ASSERT(toks.data[2] == "1.2");
		ASSERT(toks.data[3] == "\"\"");
		ASSERT(toks.data[4] == ")");
	}

	return 0;
}

#endif

