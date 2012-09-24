#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
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

    if (msgrcv(mqid, &buf, sizeof( mensaje ), 0, IPC_NOWAIT) == -1) {
        perror("msgrcv");
        return 1;
    }

    cout << buf.info.str << endl;
    return 0;
}
