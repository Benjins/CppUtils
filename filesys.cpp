#include "filesys.h"
#include "assert.cpp"

#include <stdio.h>
#include <string.h>

#if defined(__GNUC__)
#include <dirent.h>
#elif defined(_WIN32)
#endif


void File::Load(const char* path){
	struct dirent *entry;
    DIR *dp;

	snprintf(fullName, sizeof(fullName), "%s", path);
	printf("%s\n", fullName);

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




#if defined(FILESYS_TEST_MAIN)

int main(int argc, char** argv){

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

	ASSERT(maxChildCount == 3);

	return 0;
}


#endif










