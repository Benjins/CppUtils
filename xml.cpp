#include "xml.h"


XMLError ParseXMLString(String& xmlString, XMLDoc* outDoc) {
	Vector<SubString> tokens;

	XMLError err = LexXMLString(xmlString, &tokens);

	if (err != XMLE_NONE) {
		return err;
	}

	enum ParseState {
		PS_TAGSTART,
		PS_ATTRIBUTES
	};

	ParseState currentState = PS_TAGSTART;

	Vector<uint32> elemIdStack;

	for (int i = 0; i < tokens.count; i++) {
		switch (currentState) {
		
		case PS_TAGSTART:{
			if (tokens.Get(i) == "<") {
				i++;
				
				if (tokens.Get(i) == "/") {
					i++;
					SubString tokenName = tokens.Get(i);
					
					if (elemIdStack.count <= 0) {
						return XMLE_PARSINGERROR;
					}
					
					XMLElement* elem = outDoc->elements.GetById(elemIdStack.data[elemIdStack.count - 1]);

					if (elem == nullptr || elem->name != tokenName) {
						return XMLE_PARSINGERROR;
					}

					elemIdStack.PopBack();

					i++;
					if (tokens.Get(i) != ">") {
						return XMLE_PARSINGERROR;
					}
				}
				else {
					XMLElement* elem = outDoc->elements.CreateAndAdd();
					elem->doc = outDoc;
					elem->name = tokens.Get(i);

					if (elemIdStack.count > 0) {
						XMLElement* parentElem = outDoc->elements.GetById(elemIdStack.data[elemIdStack.count - 1]);
						parentElem->childrenIds.PushBack(elem->id);
					}

					elemIdStack.PushBack(elem->id);

					currentState = PS_ATTRIBUTES;
				}
			}
			else if (tokens.Get(i).start[0] == '"' || tokens.Get(i).start[0] == '\'') {
				uint32 topId = elemIdStack.data[elemIdStack.count - 1];
				outDoc->elements.GetById(topId)->plainText = tokens.Get(i);
			}
			else {
				return XMLE_PARSINGERROR;
			}
		} break;

		case PS_ATTRIBUTES: {
			if (tokens.Get(i) == ">") {
				currentState = PS_TAGSTART;
			}
			else if (tokens.Get(i) == "/") {
				elemIdStack.PopBack();
				i++;
				currentState = PS_TAGSTART;
			}
			else {
				if (i > tokens.count - 3) {
					return XMLE_PARSINGERROR;
				}

				SubString value = tokens.Get(i+2);
				value.start++;
				value.length -= 2;

				uint32 topId = elemIdStack.data[elemIdStack.count - 1];
				outDoc->elements.GetById(topId)->attributes.Insert(tokens.Get(i), value);

				i += 2;
			}
		} break;
		
		}
	}

	return XMLE_NONE;
}

XMLError ParseXMLStringFromFile(const char* fileName, XMLDoc* outDoc) {
	String str = ReadStringFromFile(fileName);

	if (str.string == nullptr) {
		return XMLE_FILENOTFOUND;
	}

	XMLError err = ParseXMLString(str, outDoc);
	short* ref = (short*)(str.string - 6);
	(*ref)--;
	
	return err;
}

XMLError LexXMLString(String& xmlString, Vector<SubString>* outTokens) {

	enum LexerState {
		LS_WHITESPACE,
		LS_SINGLESTRING,
		LS_DOUBLESTRING,
		LS_KEYSYMBOL,
		LS_IDENTIFIER
	};

	static const char* whitespace = " \t\n\r";
	static const char* keySymbols = "<>/=";

	LexerState currState = LS_WHITESPACE;

	int lastTokenStart = 0;

#define EMIT_TOKEN() {outTokens->PushBack(xmlString.GetSubString(lastTokenStart, i - lastTokenStart)); lastTokenStart = i;}

	int strLength = xmlString.GetLength();
	for (int i = 0; i < strLength; i++) {
		char currChar = xmlString.string[i];
		bool isWhiteSpace = (strchr(whitespace, currChar) != nullptr);
		bool isKeySymbol = (strchr(keySymbols, currChar) != nullptr);

		switch (currState) {

		case LS_WHITESPACE: {
			if (currChar == '\'') {
				currState = LS_SINGLESTRING;
			}
			else if (currChar == '"'){
				currState = LS_DOUBLESTRING;
			}
			else if (isKeySymbol) {
				i--;
				currState = LS_KEYSYMBOL;
			}
			else if (!isWhiteSpace) {
				currState = LS_IDENTIFIER;
			}
			else {
				lastTokenStart = i + 1;
			}
		} break;

		case LS_SINGLESTRING: {
			if (currChar == '\'') {
				i++;
				EMIT_TOKEN();
				i--;
				currState = LS_WHITESPACE;
			}
			else if (currChar == '\\') {
				i++;
			}
		} break;

		case LS_DOUBLESTRING: {
			if (currChar == '"') {
				i++;
				EMIT_TOKEN();
				i--;
				currState = LS_WHITESPACE;
			}
			else if (currChar == '\\') {
				i++;
			}
		} break;

		case LS_KEYSYMBOL: {
			i++;
			EMIT_TOKEN();
			i--;
			currState = LS_WHITESPACE;
		} break;

		case LS_IDENTIFIER: {
			if (isWhiteSpace) {
				EMIT_TOKEN();
				currState = LS_WHITESPACE;
				lastTokenStart++;
			}
			else if (isKeySymbol){
				EMIT_TOKEN();
				currState = LS_KEYSYMBOL;
				i--;
			}
		} break;

		}
	}

	return XMLE_NONE;

#undef EMIT_TOKEN
}

XMLElement* XMLElement::GetChild(const char* name, unsigned int index /*= 0*/) {
	for (int i = 0; i < childrenIds.count; i++) {
		XMLElement* child = doc->elements.GetById(childrenIds.Get(i));
		if (child->name == name) {
			if (index == 0) {
				return child;
			}
			else {
				index--;
			}
		}
	}

	return nullptr;
}

#if defined(XML_TEST_MAIN)

int main(int argc, char** argv) {
	BNS_UNUSED(argc);
	BNS_UNUSED(argv);

	String xmlStr1 = "<tag attr='valval'></tag>";
	String xmlStr2 = "<  tag attr =   'val22'>\"This is me talkign sgnkjsngkj '''\"</tag>";
	String xmlStr3 = "<tag attr1='VWG' attr2='False'><flur syn='Bah'/></tag>";

	Vector<SubString> lex1;
	Vector<SubString> lex2;
	Vector<SubString> lex3;

	LexXMLString(xmlStr1, &lex1);
	LexXMLString(xmlStr2, &lex2);
	LexXMLString(xmlStr3, &lex3);

	XMLDoc doc1;
	ParseXMLString(xmlStr1, &doc1);

	XMLDoc doc3;
	ParseXMLString(xmlStr3, &doc3);

	ASSERT(doc3.elements.currentCount == 2);

	ASSERT(doc3.elements.GetById(0)->childrenIds.count == 1);

	uint32 childId = doc3.elements.GetById(0)->childrenIds.Get(0);
	ASSERT(doc3.elements.GetById(childId) != nullptr);
	ASSERT(doc3.elements.GetById(childId)->name == "flur");
	String attrVal;
	ASSERT(doc3.elements.GetById(childId)->attributes.LookUp("syn", &attrVal) && attrVal == "Bah");

	String xmmlMatStr =
		"<Material vs='posCol.vs' fs='texCol.fs'>\n"
		"\t<uniform name='mainTex' type='tex2D' val='scribble.bmp'/>\n"
		"\t<uniform name='tintCol' type='vec4' val='0.4,0.7,0.3,1.0'/>\n"
		"</Material>\n";

	XMLDoc doc4;
	ParseXMLString(xmmlMatStr, &doc4);

	ASSERT(doc4.elements.GetById(0)->attributes.count == 2);
	ASSERT(doc4.elements.GetById(0)->childrenIds.count == 2);
	String vsVal;
	ASSERT(doc4.elements.GetById(0)->attributes.LookUp("vs", &vsVal) && vsVal == "posCol.vs");
	String fsVal;
	ASSERT(doc4.elements.GetById(0)->attributes.LookUp("fs", &fsVal) && fsVal == "texCol.fs");

	uint32 uniformId = doc4.elements.GetById(0)->childrenIds.Get(0);
	ASSERT(doc4.elements.GetById(uniformId)->attributes.count == 3);

	String typeVal;
	ASSERT(doc4.elements.GetById(uniformId)->attributes.LookUp("type", &typeVal) && typeVal == "tex2D");

	uint32 uniformId2 = doc4.elements.GetById(0)->childrenIds.Get(1);
	ASSERT(doc4.elements.GetById(uniformId2)->attributes.count == 3);

	String typeVal2;
	ASSERT(doc4.elements.GetById(uniformId2)->attributes.LookUp("type", &typeVal2) && typeVal2 == "vec4");

	return 0;
}


#endif
