#ifndef XML_H
#define XML_H

#pragma once

#include "vector.h"
#include "strings.h"
#include "stringmap.h"
#include "idbase.h"

struct XMLDocument;

struct XMLElement : IDBase {
	XMLDocument* doc;
	SubString name;
	SubString plainText;
	StringMap<String> attributes;

	Vector<uint32> childrenIds;

	XMLElement() {
		doc = nullptr;
	}

	XMLElement(const XMLElement& other) 
		: IDBase(other), name(other.name), attributes(other.attributes), childrenIds(other.childrenIds), plainText(other.plainText){
		doc = other.doc;
	}
};

struct XMLDocument {
	IDTracker<XMLElement> elements;
};

enum XMLError {
	XMLE_NONE,
	XMLE_FILENOTFOUND,
	XMLE_LEXINGERROR,
	XMLE_PARSINGERROR
};

XMLError ParseXMLString(String& xmlString, XMLDocument* outDoc);

XMLError ParseXMLStringFromFile(const char* fileName, XMLDocument* outDoc);

XMLError LexXMLString(String& xmlString, Vector<SubString>* outTokens);

#endif
