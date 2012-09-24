#include <windows.h>

typedef struct dirent {
	char d_name[100];
} DIRENT;
typedef struct {
	int first_read;
	WIN32_FIND_DATA fd;
	HANDLE fh;
	DIRENT file;
} DIR;

DIR* opendir(const char*);

struct dirent* readdir(DIR* dir);
void closedir(DIR*);
