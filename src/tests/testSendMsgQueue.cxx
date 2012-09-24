#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

using namespace std;

typedef struct un_mensaje {
    char str[20];
} mensaje;

typedef struct pirate_msgbuf {
    long mtype;  /* must be positive */
    mensaje info;
} pirate;

int main(int argc, char** argv) {
    key_t key = ftok(argv[1],1);
    int mqid = msgget(key, 0666 | IPC_CREAT);

    pirate buf;
    buf.mtype = 1;
    strncpy(buf.info.str,argv[2],20);
    buf.info.str[20] = '\0';
    
    if (msgsnd(mqid,&buf, sizeof( mensaje ), 0))
        perror("msgsnd");

    return 0;
}
