#ifndef FILESYS_H
#define FILESYS_H

#pragma once

struct File{
	char fullName[256];
	File* children;
	int childCount;

	File() {
		children = nullptr;
		childCount = 0;
	}

	void Load(const char* name);
};




#endif
