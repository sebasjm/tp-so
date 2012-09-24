#include <winsock2.h>

#include <util/log.h>
__init_log(windows_miss)

void sleep(int segs){
	Sleep(segs*1000);
}

void close(int l) {
	closesocket(l);
}

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
DIR* opendir(const char* name){
	char path[255];
	sprintf(path,"%s\\*",name);
	DIR* result = (DIR*) malloc( sizeof DIR );
	result->first_read = 1;
	result->fh = FindFirstFile(path,&result->fd);
	return result;
}

struct dirent* readdir(DIR* dir){
	if (dir->first_read){
		dir->first_read = 0;
	} else {
		if (!FindNextFile(dir->fh,&dir->fd)){
			return NULL;
		}
	}
	strcpy(dir->file.d_name,dir->fd.cFileName);
	return &dir->file;
}
void closedir(DIR* dir){
	FindClose(dir->fh);
	free(dir);
}
