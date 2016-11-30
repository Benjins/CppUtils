#ifndef XML_H
#define XML_H

#pragma once

#include "vector.h"
#include "strings.h"
#include "stringmap.h"
#include "idbase.h"

struct XMLDoc;

struct XMLElement : IDBase {
	XMLDoc* doc;
	SubString name;
	SubString plainText;
	StringMap<String> attributes;

	Vector<uint32> childrenIds;

	XMLElement() {
		doc = nullptr;
	}

	XMLElement(const XMLElement& other) 
		: IDBase(other), name(other.name), plainText(other.plainText), attributes(other.attributes), childrenIds(other.childrenIds){
		doc = other.doc;
	}

	String GetExistingAttrValue(const char* attrName);

	XMLElement* GetChild(const char* name, unsigned int index = 0);
	XMLElement* GetChildWithAttr(const char* name, const char* attrName, const char* attrValue, unsigned int index = 0);
};

struct XMLDoc {
	IDTracker<XMLElement> elements;

	XMLElement* AddElement() {
		XMLElement* elem = elements.CreateAndAdd();
		elem->doc = this;
		return elem;
	}
};

enum XMLError {
	XMLE_NONE,
	XMLE_FILENOTFOUND,
	XMLE_LEXINGERROR,
	XMLE_PARSINGERROR
};

XMLError ParseXMLString(String& xmlString, XMLDoc* outDoc);

XMLError ParseXMLStringFromFile(const char* fileName, XMLDoc* outDoc);

XMLError LexXMLString(String& xmlString, Vector<SubString>* outTokens);

XMLError SaveXMLDocToFile(XMLDoc* doc, const char* fileName);

#endif
