#ifndef FILESYS_H
#define FILESYS_H

#pragma once

struct File{
	char fullName[256];
	char* fileName;
	char* fileExt;
	File* children;
	int childCount;

	File() {
		children = nullptr;
		fileName = fullName;
		fileExt = fullName;
		childCount = 0;
	}

	void Load(const char* name);
	
	File* Find(const char* path);
};




#endif
