#ifndef LEXER_H
#define LEXER_H

#pragma once

#include "vector.h"
#include "strings.h"

Vector<SubString> LexString(String string);

int GetOperatorCount();
const char** GetOperators();

#endif
