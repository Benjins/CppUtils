#include "filesys.h"
#include "assert.h"

#include <stdio.h>
#include <string.h>

#if defined(__GNUC__)
#include <dirent.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

#if defined(__GNUC__)
void File::Load(const char* path){
	struct dirent *entry;
	DIR *dp;

	snprintf(fullName, sizeof(fullName), "%s", path);

	fileName = fullName;
	
	char* cursor = fileName;
	while(*cursor){
		if(*cursor == '/'){
			fileName = cursor+1;
		}
		
		cursor++;
	}
	
	fileExt = fileName;
	cursor = fileExt;
	while(*cursor){
		if(*cursor == '.'){
			fileExt = cursor+1;
		}
		
		cursor++;
	}
	
	dp = opendir(path);
	if (dp == NULL) {
		return;
	}

	childCount = 0;
	while ((entry = readdir(dp))){
		if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
			childCount++;
		}
	}

	closedir(dp);
	dp = opendir(path);

	if(childCount > 0){
		children = new File[childCount];
	}
	else{
		children = nullptr;
	}

	for(int i = 0; i < childCount; i++){
		entry = readdir(dp);
		if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
			char fullChildPath[512] = {0};
			snprintf(fullChildPath, 512, "%s/%s", path, entry->d_name);
			children[i].Load(fullChildPath);
		}
		else{
			i--;
		}
	}

}
#elif defined(_WIN32)
void File::Load(const char* path){
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA ffd;

	snprintf(fullName, sizeof(fullName), "%s", path);

	fileName = fullName;

	char* cursor = fileName;
	while (*cursor) {
		if (*cursor == '/') {
			fileName = cursor + 1;
		}

		cursor++;
	}

	fileExt = fileName;
	cursor = fileExt;
		while (*cursor) {
			if (*cursor == '.') {
				fileExt = cursor + 1;
			}

			cursor++;
		}

	char searchPath[512] = { 0 };
	snprintf(searchPath, 512, "%s/*", path);

	hFind = FindFirstFile(searchPath, &ffd);
	if (hFind == INVALID_HANDLE_VALUE)	{
		return;
	}

	childCount = 0;
	do {
		if (strcmp(ffd.cFileName, ".") != 0 && strcmp(ffd.cFileName, "..") != 0) {
			childCount++;
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	if(childCount > 0){
		children = new File[childCount];
	}
	else{
		children = nullptr;
	}

	hFind = FindFirstFile(searchPath, &ffd);
	for(int i = 0; i < childCount; i++){
		if (strcmp(ffd.cFileName, ".") != 0 && strcmp(ffd.cFileName, "..") != 0) {
			char fullChildPath[512] = { 0 };
			snprintf(fullChildPath, 512, "%s/%s", path, ffd.cFileName);
			children[i].Load(fullChildPath);
		}
		else {
			i--;
		}

		FindNextFile(hFind, &ffd);
	}
}
#endif

File* File::Find(const char* path){
	int dirLength = 0;
	while(path[dirLength] != '\0' && path[dirLength] != '/'){
		dirLength++;
	}
	
	for(int i = 0; i < childCount; i++){
		if(strncmp(children[i].fileName, path, dirLength) == 0){
			if(path[dirLength] == '\0'){
				return &children[i];
			}
			else{
				return children[i].Find(path + dirLength + 1);
			}
		}
	}
	
	return nullptr;
}

#if defined(FILESYS_TEST_MAIN)

int main(int argc, char** argv){
	BNS_UNUSED(argc);
	BNS_UNUSED(argv);

	File f;
	f.Load("dir1");

	ASSERT(strcmp(f.fullName, "dir1") == 0);
	ASSERT(f.childCount == 3);
	
	int maxChildCount = 0;
	for(int i = 0; i < f.childCount; i++){
		if(f.children[i].childCount > maxChildCount){
			maxChildCount = f.children[i].childCount;
		}
	}

	ASSERT(maxChildCount == 4);
	
	ASSERT(f.Find("file1.txt") != nullptr);
	ASSERT(f.Find("file1.txt")->childCount == 0);
	ASSERT(strcmp(f.Find("file1.txt")->fileName, "file1.txt") == 0);
	ASSERT(strcmp(f.Find("file1.txt")->fileExt, "txt") == 0);
	ASSERT(f.Find("dir1_c/file_c_1.txt") != nullptr);
	ASSERT(f.Find("dir1_c/dir1_c2/test.html.txt") != nullptr);
	ASSERT(strcmp(f.Find("dir1_c/dir1_c2/test.html.txt")->fileExt, "txt") == 0);
	ASSERT(strcmp(f.Find("dir1_c")->Find("dir1_c2")->Find("test.html.txt")->fileExt, "txt") == 0);

	ASSERT(strcmp(f.Find("dir1_c/dir1_c2/test.html.txt")->fullName, "dir1/dir1_c/dir1_c2/test.html.txt") == 0);
	
	return 0;
}


#endif



