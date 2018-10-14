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
	int start = (int)(tokStart - toParse.string); \
	int end = (int)(cursor - toParse.string); \
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

	if (state != LS_WhiteSpace) {
		EMIT_TOKEN();
	}

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
			if (*index >= tokens.count) {
				break;
			}
			else if (tokens.data[*index] == ")") {
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
	if (tokens.data[*index].start[0] != '"' && tokens.data[*index].start[0] != '\''
	&& tokens.data[*index] != "(" && tokens.data[*index] != ")") {
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

	bool atLeastOneDigit = false;

	for (;  i < tok.length; i++) {
		if (IsDigit(tok.start[i])){
			atLeastOneDigit = true;
		}
		else if (tok.start[i] == '.') {
			isFloat = true;
		}
		else {
			isNum = false;
			break;
		}
	}

	if (atLeastOneDigit && isNum) {
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

bool IdentifierIsSpecialMatchClause(const SubString& name, SubString* outClause) {
	if (name.length >= 3) {
		if (name.start[0] == '@' && name.start[1] == '{') {
			if (name.start[name.length - 1] == '}') {
				*outClause = name;
				outClause->start += 2;
				outClause->length -= 3;

				return true;
			}
		}
	}

	return false;
}

bool MatchSexpr(BNSexpr* sexpr, BNSexpr* matchSexpr, const Vector<BNSexpr*>& args, int* index);

bool MatchSexpr(BNSexpr* sexpr, const char* format, const Vector<BNSexpr*>& args) {
	Vector<BNSexpr> sexprs;
	BNSexprParseResult res = ParseSexprs(&sexprs, format);
	ASSERT(res == BNSexpr_Success);
	ASSERT(sexprs.count == 1);

	BNSexpr* matchSexpr = &sexprs.data[0];
	int idx = 0;
	bool isMatch = MatchSexpr(sexpr, matchSexpr, args, &idx);

	if (isMatch) {
		ASSERT(idx == args.count);
	}

	return isMatch;
}

bool MatchSexpr(BNSexpr* sexpr, BNSexpr* matchSexpr, const Vector<BNSexpr*>& args, int* index) {
	if (matchSexpr->IsBNSexprString()) {
		return sexpr->IsBNSexprString() && sexpr->AsBNSexprString().value == matchSexpr->AsBNSexprString().value;
	}
	else if (matchSexpr->IsBNSexprNumber()) {
		return sexpr->IsBNSexprNumber() && sexpr->AsBNSexprNumber() == matchSexpr->AsBNSexprNumber();
	}
	else if (matchSexpr->IsBNSexprIdentifier()) {
		SubString specialMatchClause;
		if (IdentifierIsSpecialMatchClause(matchSexpr->AsBNSexprIdentifier().identifier, &specialMatchClause)) {
#define DO_MATCH() do { BNSexpr* matched = args.Get(*index); (*index)++; *matched = *sexpr; return true; } while(0)
			if (specialMatchClause == "") {
				DO_MATCH();
			}
			else if (specialMatchClause == "num") {
				if (sexpr->IsBNSexprNumber()) {
					DO_MATCH();
				}
				else {
					return false;
				}
			}
			else if (specialMatchClause == "id") {
				if (sexpr->IsBNSexprIdentifier()) {
					DO_MATCH();
				}
				else {
					return false;
				}
			}
			else if (specialMatchClause == "str") {
				if (sexpr->IsBNSexprString()) {
					DO_MATCH();
				}
				else {
					return false;
				}
			}
			else {
				ASSERT(false);
				return false;
			}
#undef DO_MATCH
		}
		else {
			return sexpr->IsBNSexprIdentifier()
				&& sexpr->AsBNSexprIdentifier().identifier == matchSexpr->AsBNSexprIdentifier().identifier;
		}
	}
	else if (matchSexpr->IsBNSexprParenList()) {
		if (sexpr->IsBNSexprParenList()) {
			Vector<BNSexpr>& matchChildren = matchSexpr->AsBNSexprParenList().children;
			Vector<BNSexpr>& sexprChildren = sexpr->AsBNSexprParenList().children;
			if (matchChildren.count > 0
			 && matchChildren.Back().IsBNSexprIdentifier()
			 && matchChildren.Back().AsBNSexprIdentifier().identifier == "@{...}") {
				if (sexprChildren.count >= matchChildren.count - 2) {
					bool allMatch = true;
					for (int i = 0; i < matchChildren.count - 2; i++) {
						BNSexpr* sepxrChild = &sexprChildren.data[i];
						BNSexpr* matchChild = &matchChildren.data[i];
						if (!MatchSexpr(sepxrChild, matchChild, args, index)) {
							allMatch = false;
							break;
						}
					}

					BNSexprParenList matchRest;
					BNSexpr* matchChild = &matchChildren.data[matchChildren.count - 2];
					for (int i = matchChildren.count - 2; i < sexprChildren.count; i++) {
						BNSexpr* sepxrChild = &sexprChildren.data[i];
						if (!MatchSexpr(sepxrChild, matchChild, args, index)) {
							allMatch = false;
						}
						else {
							(*index)--;
							matchRest.children.PushBack(*args.Get(*index));
						}
					}

					if (allMatch) {
						(*index)++;
						*(args.data[(*index) - 1]) = matchRest;
						return true;
					}
					else {
						return false;
					}
				}
				else {
					return false;
				}
			}
			else if (sexprChildren.count == matchChildren.count) {
				bool allMatch = true;
				for (int i = 0; i < sexprChildren.count; i++) {
					BNSexpr* sepxrChild = &sexprChildren.data[i];
					BNSexpr* matchChild = &matchChildren.data[i];
					if (!MatchSexpr(sepxrChild, matchChild, args, index)) {
						allMatch = false;
						break;
					}
				}

				return allMatch;
			}
		}
		return false;
	}
	else {
		ASSERT(false);
		return false;
	}
}

void PrintSexpr(BNSexpr* sexpr, FILE* outFile /*= stdout*/) {
	if (sexpr->IsBNSexprNumber()) {
		if (sexpr->AsBNSexprNumber().isFloat) {
			fprintf(outFile, "%lf", sexpr->AsBNSexprNumber().fValue);
		}
		else {
			fprintf(outFile, "%lld", sexpr->AsBNSexprNumber().iValue);
		}
	}
	else if (sexpr->IsBNSexprString()) {
		fprintf(outFile, "'%.*s'", BNS_LEN_START(sexpr->AsBNSexprString().value));
	}
	else if (sexpr->IsBNSexprIdentifier()) {
		fprintf(outFile, "%.*s", BNS_LEN_START(sexpr->AsBNSexprIdentifier().identifier));
	}
	else if (sexpr->IsBNSexprParenList()) {
		fprintf(outFile, "(");
		bool isFirst = true;
		BNS_VEC_FOREACH(sexpr->AsBNSexprParenList().children) {
			if (!isFirst) {
				fprintf(outFile, " ");
			}
			PrintSexpr(ptr);

			isFirst = false;
		}
		fprintf(outFile, ")");
	}
	else {
		ASSERT(false);
	}
}

#if defined(BNSEXPR_TEST_MAIN)

CREATE_TEST_CASE("Sexpr") {

	// Test Sexpr lexing
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
		Vector<SubString> toks = BNSexprLexTokens("() ");

		ASSERT(toks.count == 2);
		ASSERT(toks.data[0] == "(");
		ASSERT(toks.data[1] == ")");
	}

	{
		Vector<SubString> toks = BNSexprLexTokens("()  \"\" ");

		ASSERT(toks.count == 3);
		ASSERT(toks.data[0] == "(");
		ASSERT(toks.data[1] == ")");
		ASSERT(toks.data[2] == "\"\"");
	}

	{
		Vector<SubString> toks = BNSexprLexTokens("(   \t\t  \n\r\n)\n\r\t  \t\n");

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

	// Test Sexpr parsing
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
		BNSexprParseResult res = ParseSexprs(&sexprs, "(yo 'hello')");
		ASSERT(res == BNSexpr_Success);
	}
	
	{
		Vector<BNSexpr> sexprs;
		BNSexprParseResult res = ParseSexprs(&sexprs, "(yo 'hello'))");
		ASSERT(res == BNSexpr_Error);
	}
	
	{
		Vector<BNSexpr> sexprs;
		BNSexprParseResult res = ParseSexprs(&sexprs, "(yo 'hello'");
		ASSERT(res == BNSexpr_Error);
	}

	{
		Vector<BNSexpr> sexprs;
		BNSexprParseResult res = ParseSexprs(&sexprs, ")(");
		ASSERT(res == BNSexpr_Error);
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
		ParseSexprs(&sexprs, "---");

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
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "...");

		ASSERT(sexprs.count == 1);
		ASSERT(sexprs.data[0].type == BNSexpr::UE_BNSexprIdentifier);
	}

	// Test Sexpr matching
	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(this is me)");	

		BNSexpr id1, id2, id3;
		bool res = MatchSexpr(&sexprs.data[0], "(@{id} @{id} @{id})", { &id1, &id2, &id3 } );

		ASSERT(res == true);
		ASSERT(id1.IsBNSexprIdentifier());
		ASSERT(id2.IsBNSexprIdentifier());
		ASSERT(id3.IsBNSexprIdentifier());
		ASSERT(id1.AsBNSexprIdentifier().identifier == "this");
		ASSERT(id2.AsBNSexprIdentifier().identifier == "is");
		ASSERT(id3.AsBNSexprIdentifier().identifier == "me");
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "'yo'");

		BNSexpr str;
		bool res = MatchSexpr(&sexprs.data[0], "@{str}", { &str });
		ASSERT(res == true);
		ASSERT(str.IsBNSexprString());
		ASSERT(str.AsBNSexprString().value == "'yo'");
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "'yo'");

		BNSexpr str;
		bool res = MatchSexpr(&sexprs.data[0], "@{}", { &str });
		ASSERT(res == true);
		ASSERT(str.IsBNSexprString());
		ASSERT(str.AsBNSexprString().value == "'yo'");
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "'yo'");

		BNSexpr str;
		bool res = MatchSexpr(&sexprs.data[0], "@{id}", { &str });
		ASSERT(res == false);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "'yo'");

		BNSexpr str;
		bool res = MatchSexpr(&sexprs.data[0], "@{num}", { &str });
		ASSERT(res == false);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "4561");

		BNSexpr num;
		bool res = MatchSexpr(&sexprs.data[0], "@{num}", { &num });
		ASSERT(res == true);
		ASSERT(num.IsBNSexprNumber());
		ASSERT(!num.AsBNSexprNumber().isFloat);
		ASSERT(num.AsBNSexprNumber().iValue == 4561);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(hello (blah 23 45) 'erg')");

		BNSexpr num1, num2, str;
		bool res = MatchSexpr(&sexprs.data[0], "(hello (blah @{num} @{num}) @{str})", { &num1, &num2, &str });
		ASSERT(res == true);
		ASSERT(num1.IsBNSexprNumber());
		ASSERT(num2.IsBNSexprNumber());
		ASSERT(num1.AsBNSexprNumber() == 23);
		ASSERT(num2.AsBNSexprNumber() == 45);
		ASSERT(str.IsBNSexprString());
		ASSERT(str.AsBNSexprString().value == "'erg'");
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(hello (blah 23 45) 'erg')");

		BNSexpr num1, num2, str;
		bool res = MatchSexpr(&sexprs.data[0], "(hello (blah @{} @{}) @{})", { &num1, &num2, &str });
		ASSERT(res == true);
		ASSERT(num1.IsBNSexprNumber());
		ASSERT(num2.IsBNSexprNumber());
		ASSERT(num1.AsBNSexprNumber() == 23);
		ASSERT(num2.AsBNSexprNumber() == 45);
		ASSERT(str.IsBNSexprString());
		ASSERT(str.AsBNSexprString().value == "'erg'");
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(hello (blah 23 45) 'erg')");

		BNSexpr num1, num2, str;
		bool res = MatchSexpr(&sexprs.data[0], "(hello (blah @{id} @{num}) @{str})", { &num1, &num2, &str });
		ASSERT(res == false);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(hello (blah 23 45) 'erg')");

		BNSexpr num1, num2, str;
		bool res = MatchSexpr(&sexprs.data[0], "(hello (blah (@{num}) @{num}) @{str})", { &num1, &num2, &str });
		ASSERT(res == false);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(hello (blah 23 45) 'erg')");

		BNSexpr num1, num2, str;
		bool res = MatchSexpr(&sexprs.data[0], "(hello (@{num} blah @{num}) @{str})", { &num1, &num2, &str });
		ASSERT(res == false);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "()");

		BNSexpr paren;
		bool res = MatchSexpr(&sexprs.data[0], "@{}", { &paren });
		ASSERT(res == true);
		ASSERT(paren.IsBNSexprParenList());
		ASSERT(paren.AsBNSexprParenList().children.count == 0);
	}
	
	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "('yes' 'no')");

		bool res = MatchSexpr(&sexprs.data[0], "('yes' 'no')", {});
		ASSERT(res == true);
	}
	
	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "('yes' 5)");

		BNSexpr str;
		bool res = MatchSexpr(&sexprs.data[0], "('yes' @{str})", {&str});
		ASSERT(res == false);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(+ (* 2 3) (* 6 5))");

		int expected[4] = { 2, 3, 6, 5 };
		BNSexpr nums[4];
		bool res = MatchSexpr(&sexprs.data[0], "(+ (* @{num} @{num}) (* @{num} @{num}))", 
								{ &nums[0], &nums[1], &nums[2], &nums[3] });
		ASSERT(res == true);
		for (int i = 0; i < 4; i++) {
			ASSERT(nums[i].IsBNSexprNumber());
			ASSERT(!nums[i].AsBNSexprNumber().isFloat);
			ASSERT(nums[i].AsBNSexprNumber().iValue == expected[i]);
		}
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(+ (* 2 3) (* 6 5))");

		BNSexpr subExpr, num1, num2;
		bool res = MatchSexpr(&sexprs.data[0], "(+ @{} (* @{num} @{num}))", { &subExpr, &num1, &num2});
		ASSERT(res == true);
		ASSERT(subExpr.IsBNSexprParenList());
		ASSERT(subExpr.AsBNSexprParenList().children.count == 3);
		ASSERT(num1.IsBNSexprNumber());
		ASSERT(num2.IsBNSexprNumber());
		ASSERT(num1.AsBNSexprNumber() == 6);
		ASSERT(num2.AsBNSexprNumber() == 5);

		BNSexpr subNum;
		bool res2 = MatchSexpr(&subExpr, "(* @{num} 3)", { &subNum });
		ASSERT(res2 == true);
		ASSERT(subNum.IsBNSexprNumber());
		ASSERT(subNum.AsBNSexprNumber() == 2);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(+ (* 2 3) (* 6 5))");

		bool res = MatchSexpr(&sexprs.data[0], "(+ (* 2 3) (* 6 5))", {});
		ASSERT(res == true);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(+ 2 3 4 5)");

		BNSexpr nums;
		bool res = MatchSexpr(&sexprs.data[0], "(+ @{num} @{...})", { &nums });
		ASSERT(res == true);
		ASSERT(nums.IsBNSexprParenList());
		ASSERT(nums.AsBNSexprParenList().children.count == 4);
		for (int i = 0; i < 4; i++) {
			ASSERT(nums.AsBNSexprParenList().children.data[i].IsBNSexprNumber());
			ASSERT(nums.AsBNSexprParenList().children.data[i].AsBNSexprNumber() == i + 2);
		}
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(state my-name (option 'yo' => high) (option \"no\" => low) (option 'what' => unknown))");

		BNSexpr stateName, options;
		bool res = MatchSexpr(&sexprs.data[0], "(state @{id} @{} @{...})", { &stateName, &options });
		ASSERT(res == true);
		ASSERT(stateName.IsBNSexprIdentifier());
		ASSERT(stateName.AsBNSexprIdentifier().identifier == "my-name");
		ASSERT(options.IsBNSexprParenList());
		ASSERT(options.AsBNSexprParenList().children.count == 3);

		const char* diags[3] = { "'yo'", "\"no\"", "'what'" };
		const char* newStates[3] = {"high", "low", "unknown"};
		for (int i = 0; i < 3; i++) {
			BNSexpr str, newState;
			BNSexpr* subSexpr = &options.AsBNSexprParenList().children.data[i];
			bool subRes = MatchSexpr(subSexpr, "(option @{str} => @{id})", {&str, &newState});
			ASSERT(subRes == true);
			ASSERT(str.IsBNSexprString());
			ASSERT(str.AsBNSexprString().value == diags[i]);
			ASSERT(newState.IsBNSexprIdentifier());
			ASSERT(newState.AsBNSexprIdentifier().identifier == newStates[i]);
		}
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(sum)");

		BNSexpr nums;
		bool res = MatchSexpr(&sexprs.data[0], "(sum @{num} @{...})", { &nums });
		ASSERT(nums.IsBNSexprParenList());
		ASSERT(nums.AsBNSexprParenList().children.count == 0);
	}


	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(sum the nums)");

		BNSexpr nums;
		bool res = MatchSexpr(&sexprs.data[0], "(sum the nums @{num} @{...})", { &nums });
		ASSERT(nums.IsBNSexprParenList());
		ASSERT(nums.AsBNSexprParenList().children.count == 0);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(sum 3)");

		BNSexpr nums;
		bool res = MatchSexpr(&sexprs.data[0], "(sum @{num} @{...})", { &nums });
		ASSERT(nums.IsBNSexprParenList());
		ASSERT(nums.AsBNSexprParenList().children.count == 1);
		ASSERT(nums.AsBNSexprParenList().children.data[0].IsBNSexprNumber());
		ASSERT(nums.AsBNSexprParenList().children.data[0].AsBNSexprNumber() == 3);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(sum the nums 3)");

		BNSexpr nums;
		bool res = MatchSexpr(&sexprs.data[0], "(sum the nums @{num} @{...})", { &nums });
		ASSERT(nums.IsBNSexprParenList());
		ASSERT(nums.AsBNSexprParenList().children.count == 1);
		ASSERT(nums.AsBNSexprParenList().children.data[0].IsBNSexprNumber());
		ASSERT(nums.AsBNSexprParenList().children.data[0].AsBNSexprNumber() == 3);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(sum the)");

		BNSexpr nums;
		bool res = MatchSexpr(&sexprs.data[0], "(sum the nums @{num} @{...})", { &nums });
		ASSERT(res == false);
	}
	
	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(sum the nums 0 3 4 er 5)");

		BNSexpr nums;
		bool res = MatchSexpr(&sexprs.data[0], "(sum the nums @{num} @{...})", { &nums });
		ASSERT(res == false);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(sum the nums 0 3 4 (4 5 3) 5)");

		BNSexpr nums;
		bool res = MatchSexpr(&sexprs.data[0], "(sum the nums @{num} @{...})", { &nums });
		ASSERT(res == false);
	}
	
	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(sum the nums 0 3 4 '34' 5)");

		BNSexpr nums;
		bool res = MatchSexpr(&sexprs.data[0], "(sum the nums @{num} @{...})", { &nums });
		ASSERT(res == false);
	}
	
	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(sum YES nums 0 3 4 5)");

		BNSexpr nums;
		bool res = MatchSexpr(&sexprs.data[0], "(sum the nums @{num} @{...})", { &nums });
		ASSERT(res == false);
	}
	
	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(sum YES nums 0 3 4 5)");

		BNSexpr nums;
		bool res = MatchSexpr(&sexprs.data[0], "(sum the nums @{num} @{...})", { &nums });
		ASSERT(res == false);
	}
	
	{
		const char* sexprText =
		"(row (name r1)                               \
			(padding 10)                              \
			(spacing 20)                              \
			(elements b1 b2 b3)                       \
			(height (* 2 (height h)))                 \
			(width (- (width page) (/ (* 3 (x h)) 2))))";
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, sexprText);
		ASSERT(sexprs.count == 1);
		BNSexpr name, padding, spacing, elems, heightExpr, widthExpr;
		bool res = MatchSexpr(&sexprs.data[0], 
				"(row (name @{id}) (padding @{num}) (spacing @{num}) (elements @{id} @{...}) (height @{}) (width @{}))", 
				{ &name, &padding, &spacing, &elems, &heightExpr, &widthExpr });

		ASSERT(res == true);
		ASSERT(name.IsBNSexprIdentifier());
		ASSERT(name.AsBNSexprIdentifier().identifier == "r1");
		ASSERT(padding.IsBNSexprNumber());
		ASSERT(padding.AsBNSexprNumber() == 10);
		ASSERT(spacing.IsBNSexprNumber());
		ASSERT(spacing.AsBNSexprNumber() == 20);
		ASSERT(elems.IsBNSexprParenList());
		ASSERT(elems.AsBNSexprParenList().children.count == 3);
		const char* expectedIds[3] = { "b1", "b2", "b3" };
		for (int i = 0; i < 3; i++) {
			ASSERT(elems.AsBNSexprParenList().children.data[i].IsBNSexprIdentifier());
			ASSERT(elems.AsBNSexprParenList().children.data[i].AsBNSexprIdentifier().identifier == expectedIds[i]);
		}
		ASSERT(heightExpr.IsBNSexprParenList());
		ASSERT(widthExpr.IsBNSexprParenList());
		ASSERT(heightExpr.AsBNSexprParenList().children.count == 3);
		ASSERT(widthExpr.AsBNSexprParenList().children.count == 3);
	}

	{
		Vector<BNSexpr> sexprs;
		ParseSexprs(&sexprs, "(blah x y z)");
		ASSERT(sexprs.count == 1);

		BNSexpr func, args;
		bool res = MatchSexpr(&sexprs.data[0], "(@{id} @{id} @{...})", {&func, &args});
		ASSERT(res == true);
		ASSERT(func.IsBNSexprIdentifier());
		ASSERT(func.AsBNSexprIdentifier().identifier == "blah");
		ASSERT(args.IsBNSexprParenList());
		ASSERT(args.AsBNSexprParenList().children.count == 3);
		const char* expectedIds[3] = { "x", "y", "z" };
		for (int i = 0; i < 3; i++) {
			ASSERT(args.AsBNSexprParenList().children.data[i].IsBNSexprIdentifier());
			ASSERT(args.AsBNSexprParenList().children.data[i].AsBNSexprIdentifier().identifier == expectedIds[i]);
		}
	}

	return 0;
}

#endif

