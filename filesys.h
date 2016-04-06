#ifndef FILESYS_H
#define FILESYS_H

#pragma once

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
	
	File* Find(const char* path);
};




#endif
