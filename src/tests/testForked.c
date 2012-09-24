/*
 * testForked.c
 *
 *  Created on: 23/11/2010
 *      Author: pmarchesi
 */
#include "node/dc/dc.h"
#include "ipc/ipcDaemon.h"
#include "messages/ipc-dc-nodo.h"
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <util/log_ansi.h>
#define modulo "testForked"
__init_logname(modulo)

int main(int argc, char** argv) {
	strncpy(argv[0], "DC", strlen(argv[0]));

	key_t semkey = ftok(argv[1], 0666 | IPC_CREAT);
	fprintf(stderr, "KEYMsgQueueParent %d\n", semkey);
	if (semkey == -1) {
		perror("SemKetTODC");
	}

	int msgidToDC = msgget(semkey, 0666 | IPC_CREAT);
	fprintf(stderr, "MsgQueueFork %d\n", msgidToDC);

	while (1) {

		IHeader* header = (IHeader*) malloc(sizeof(IHeader));
		int rta = readMessage(msgidToDC, header, sizeof(IHeader));
		fprintf(stderr, "Empiezo a leer mensaje Mensaje (%d) %s\n", rta, header->id);
		switch (header->payload) {
		case '1': {
			Content1* content = (Content1*) malloc(sizeof(Content1));
			readMessage(msgidToDC, content, sizeof(Content1));
			fprintf(stderr, "Num1:%ld  Num2:%d\n", content->num1, content->num2);
		}
			break;
		case '2': {
			Content2* content2 = (Content2*) malloc(sizeof(Content2));
			readMessage(msgidToDC, content2, sizeof(Content2));
			fprintf(stderr, "Num:%d\n", content2->num);
		}
			break;

		default:
			fprintf(stderr, "ERROR %s.\n", header->id);
			break;
		}

	}
	int migration;
	if (argv[1] == NULL) {
		migration = 0;
	} else {
		migration = atoi(argv[1]);
	}
	startDC(migration);

}
