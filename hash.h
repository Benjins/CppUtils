#ifndef HASH_H
#define HASH_H

#pragma once

typedef unsigned int Hash;

Hash ComputeHash(const void* mem, int size);

Hash ComputeHash(const char* str);


#endif