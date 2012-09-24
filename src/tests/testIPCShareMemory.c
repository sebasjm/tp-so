#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "ipc/ipcDaemon.h"
#include "messages/ipc-dc-nodo.h"
#include <util/config.h>
#include <signal.h>

int msgidToDC;
int msgidToNodo;
void salir(int sig) {
	exit(0);
}
void interrupt() {
	struct sigaction action;

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);

	sigemptyset(&action.sa_mask);

	action.sa_flags = 0;
	action.sa_handler = salir;
	sigaction(SIGUSR1, &action, NULL);

	sigprocmask(SIG_BLOCK, &set, NULL);
}

char* createQueues(char** argv) {
	char* childBin = (char*) malloc(sizeof(read_config("./config/daemon.xml", "dc_main.bin")) + 1);
	childBin = read_config("./config/daemon.xml", "dc_main.bin");
	key_t semkeyToDC;

	semkeyToDC = ftok(argv[0], 0666 | IPC_CREAT);
	fprintf(stderr, "KEYMsgQueueParent %d\n", semkeyToDC);
	if (semkeyToDC == -1) {
		perror("SemKetTODC");
	}
	msgctl(msgget(semkeyToDC, 0666 | IPC_CREAT), IPC_RMID, NULL); /* Vaciar Cola*/

	msgidToDC = msgget(semkeyToDC, 0666 | IPC_CREAT);
	fprintf(stderr, "MsgQueueClean %d\n", msgidToDC);

	semkeyToDC = ftok(childBin, 0666 | IPC_CREAT);
	fprintf(stderr, "KEYMsgQueueParent %d\n", semkeyToDC);
	if (semkeyToDC == -1) {
		perror("SemKetTODC");
	}
	msgctl(msgget(semkeyToDC, 0666 | IPC_CREAT), IPC_RMID, NULL); /* Vaciar Cola*/

	msgidToNodo = msgget(semkeyToDC, 0666 | IPC_CREAT);
	fprintf(stderr, "MsgQueueClean %d\n", msgidToNodo);
	return childBin;

}
int main(int argc, char** argv) {
	pid_t childpid;
	interrupt();

	char* childBin = createQueues(argv);

	if ((childpid = fork()) < 0) {
		perror("Fork Failed");
		exit(1);
	} else if (childpid != 0) {
		char* arg[4];
		arg[0] = childBin;
		arg[1] = argv[0];
		arg[2] = "0";
		arg[3] = NULL;
		setsid();
		execv(arg[0], arg);
	} else {
		strncpy(argv[0], "Nodo", strlen(argv[0]));
		int i = 0;
		fprintf(stderr, "MsgQueueParent %d\n", msgidToDC);
		while (1) {
			if ((i % 2) == 0) {
				Message1* message = (Message1*) malloc(sizeof(Message1));
				strcpy(message->header.id, "1");
				message->header.payload = '1';
				message->content.num1 = 12361236 + i;
				message->content.num2 = 12361 + i;
				sendMessage(msgidToDC, &message->header, &message->content, sizeof(message->content));
			} else {
				Message2* message = (Message2*) malloc(sizeof(Message2));
				strcpy(message->header.id, "2");
				message->header.payload = '2';
				message->content.num = 7423 + i;
				sendMessage(msgidToDC, &message->header, &message->content, sizeof(message->content));
			}
			sleep(2);
			i++;
			if (i == 15)
				exit(0);
		}
	}

	return 0;
}
