#ifndef HASH_H
#define HASH_H

#pragma once

typedef unsigned int Hash;

Hash ComputeHash(void* mem, int size);

Hash ComputeHash(char* str);


#endif