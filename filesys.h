#ifndef FILESYS_H
#define FILESYS_H

#pragma once

#include "vector.h"

#define NOT_A_DIRECTORY -1

struct File{
	char fullName[256];
	char* fileName;
	char* fileExt;
	File* parent;
	File* children;
	int childCount;

	File() {
		children = nullptr;
		parent = nullptr;
		fullName[0] = '\0';
		fileName = fullName;
		fileExt = fullName;
		childCount = NOT_A_DIRECTORY;
	}

	void Load(const char* name);

	void Unload();
	
	File* Find(const char* path);
	void FindFilesWithExt(const char* ext, Vector<File*>* outFiles);
};

unsigned char* ReadBinaryFile(const char* fileName, int* outLength);
char* ReadTextFile(const char* fileName, int* outLength);


#endif
