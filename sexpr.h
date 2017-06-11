#ifndef BNS_SEXPR_H
#define BNS_SEXPR_H

#pragma once

#include "vector.h"
#include "strings.h"
#include "disc_union.h"

struct BNSexpr;

struct BNSexprString {
	SubString value;

	BNSexprString() { }

	BNSexprString(const SubString& _value) {
		value = _value;
	}
};

struct BNSexprNumber {
	bool isFloat;
	union {
		long long int iValue;
		double fValue;
	};

	BNSexprNumber(int _iValue) {
		iValue = _iValue;
		isFloat = false;
	}

	BNSexprNumber(long long int _iValue) {
		iValue = _iValue;
		isFloat = false;
	}

	BNSexprNumber(double _fValue) {
		fValue = _fValue;
		isFloat = true;
	}

	void operator=(long long int _iValue) {
		iValue = _iValue;
		isFloat = false;
	}

	void operator=(int _iValue) {
		iValue = _iValue;
		isFloat = false;
	}

	void operator=(double _fValue) {
		fValue = _fValue;
		isFloat = true;
	}

	bool operator==(const BNSexprNumber& other) {
		if (!isFloat && !other.isFloat) {
			return iValue == other.iValue;
		}
		else {
			return CoerceDouble() == other.CoerceDouble();
		}
	}
		
	double CoerceDouble() const {
		return isFloat ? fValue : (double)iValue;
	}
};

struct BNSexprIdentifier {
	SubString identifier;

	BNSexprIdentifier() { }

	BNSexprIdentifier(const SubString& _identifier) {
		identifier = _identifier;
	}
};

struct BNSexprParenList {
	Vector<BNSexpr> children;
};

#define DISC_LIST(mac) \
	mac(BNSexprNumber) \
	mac(BNSexprIdentifier) \
	mac(BNSexprParenList) \
	mac(BNSexprString)

// This generates the actual struct body
DEFINE_DISCRIMINATED_UNION(BNSexpr, DISC_LIST)

#undef DISC_LIST

enum BNSexprParseResult {
	BNSexpr_Success,
	BNSexpr_Error
};

BNSexprParseResult ParseSexprs(Vector<BNSexpr>* outSexprs, const String& str);

bool MatchSexpr(BNSexpr* sexpr, const char* format, const Vector<BNSexpr*>& args);

void PrintSexpr(BNSexpr* sexpr, FILE* outFile = stdout);

#endif
