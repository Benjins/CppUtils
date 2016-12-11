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

	Vector<IDHandle<XMLElement>> elemIdStack;

	for (int i = 0; i < tokens.count; i++) {
		switch (currentState) {
		
		case PS_TAGSTART:{
			if (tokens.Get(i) == "<") {
				if (i < tokens.count - 1 && tokens.Get(i + 1) == "?xml") {
					int j = i + 1;
					while (j < tokens.count - 2 && (tokens.Get(j) != "?" || tokens.Get(j+1) != ">")) {
						j++;
					}
					i = j + 1;
				}
				else {
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
							parentElem->childrenIds.PushBack(IDHandle<XMLElement>(elem->id));
						}

						elemIdStack.PushBack(IDHandle<XMLElement>(elem->id));

						currentState = PS_ATTRIBUTES;
					}
				}
			}
			else if (tokens.Get(i).start[0] == '"' || tokens.Get(i).start[0] == '\'') {
				IDHandle<XMLElement> topId = elemIdStack.data[elemIdStack.count - 1];
				outDoc->elements.GetById(topId)->plainText = tokens.Get(i);
			}
			else {
				int j = i;
				while (j < tokens.count) {
					if (tokens.Get(j) == "<") {
						break;
					}
					j++;
				}

				if (j == tokens.count) {
					return XMLE_PARSINGERROR;
				}

				char* sp = tokens.Get(i).start;
				char* ep = tokens.Get(j - 1).start + tokens.Get(j - 1).length;
				int idx = sp - xmlString.string;
				int len = ep - sp;

				IDHandle<XMLElement> topId = elemIdStack.data[elemIdStack.count - 1];
				outDoc->elements.GetById(topId)->plainText = xmlString.GetSubString(idx, len);
				i = j - 1;
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

				IDHandle<XMLElement> topId = elemIdStack.data[elemIdStack.count - 1];
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

String XMLElement::GetExistingAttrValue(const char* attrName) {
	String val;
	bool found = attributes.LookUp(attrName, &val);

	ASSERT_MSG(found, "XMLElement of type '%.*s' is missing attribute '%s'", name.length, name.start, attrName);

	return val;
}

XMLElement* XMLElement::GetChildWithAttr(const char* name, const char* attrName, const char* attrValue, unsigned int index /*= 0*/) {
	for (int i = 0; i < childrenIds.count; i++) {
		XMLElement* child = doc->elements.GetById(childrenIds.Get(i));
		if (child->name == name) {
			String attrib;
			bool foundAttr = child->attributes.LookUp(attrName, &attrib);
			if (foundAttr && attrib == attrValue) {
				if (index == 0) {
					return child;
				}
				else {
					index--;
				}
			}
		}
	}

	return nullptr;
}

void WriteXMLElementToFile(XMLElement* elem, FILE* outFile, int indentation) {

#define PRINT_INDENT(n) for(int idt = 0; idt < n; idt++){fprintf(outFile, "\t");}

	PRINT_INDENT(indentation);
	fprintf(outFile, "<%.*s", elem->name.length, elem->name.start);

	for (int i = 0; i < elem->attributes.count; i++) {
		fprintf(outFile, " %s='%s'", elem->attributes.names[i].string, elem->attributes.values[i].string);
	}

	if (elem->childrenIds.count == 0) {
		if (elem->plainText.length > 0) {
			fprintf(outFile, ">");
			fprintf(outFile, "%.*s", elem->plainText.length, elem->plainText.start);
			fprintf(outFile, "</%.*s>\n", elem->name.length, elem->name.start);
		}
		else {
			fprintf(outFile, "/>\n");
		}
	}
	else {
		fprintf(outFile, ">\n");

		for (int i = 0; i < elem->childrenIds.count; i++) {
			IDHandle<XMLElement> childId = elem->childrenIds.Get(i);
			XMLElement* childElem = elem->doc->elements.GetById(childId);
			WriteXMLElementToFile(childElem, outFile, indentation + 1);
		}

		PRINT_INDENT(indentation);
		fprintf(outFile, "</%.*s>\n", elem->name.length, elem->name.start);
	}

#undef PRINT_INDENT
}

XMLError SaveXMLDocToFile(XMLDoc* doc, const char* fileName) {
	FILE* outFile = fopen(fileName, "wb");
	if (outFile == NULL) {
		return XMLE_FILENOTFOUND;
	}

	WriteXMLElementToFile(&doc->elements.vals[0], outFile, 0);

	fclose(outFile);

	return XMLE_NONE;
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

	ASSERT(doc3.elements.GetByIdNum(0)->childrenIds.count == 1);

	IDHandle<XMLElement> childId = doc3.elements.GetByIdNum(0)->childrenIds.Get(0);
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

	XMLElement* root = &doc4.elements.vals[0];
	XMLElement* tintColElem = root->GetChildWithAttr("uniform", "name", "tintCol");
	ASSERT(tintColElem != nullptr);
	ASSERT(tintColElem->name == "uniform");

	{
		//TODO: Why isn't this working?
		//XMLDoc doc5;
		//ParseXMLStringFromFile("test1.xml", &doc5);
	}

	ASSERT(doc4.elements.GetByIdNum(0)->attributes.count == 2);
	ASSERT(doc4.elements.GetByIdNum(0)->childrenIds.count == 2);
	String vsVal;
	ASSERT(doc4.elements.GetByIdNum(0)->attributes.LookUp("vs", &vsVal) && vsVal == "posCol.vs");
	String fsVal;
	ASSERT(doc4.elements.GetByIdNum(0)->attributes.LookUp("fs", &fsVal) && fsVal == "texCol.fs");

	IDHandle<XMLElement> uniformId = doc4.elements.GetByIdNum(0)->childrenIds.Get(0);
	ASSERT(doc4.elements.GetById(uniformId)->attributes.count == 3);

	String typeVal;
	ASSERT(doc4.elements.GetById(uniformId)->attributes.LookUp("type", &typeVal) && typeVal == "tex2D");

	IDHandle<XMLElement> uniformId2 = doc4.elements.GetByIdNum(0)->childrenIds.Get(1);
	ASSERT(doc4.elements.GetById(uniformId2)->attributes.count == 3);

	String typeVal2;
	ASSERT(doc4.elements.GetById(uniformId2)->attributes.LookUp("type", &typeVal2) && typeVal2 == "vec4");

	return 0;
}


#endif
