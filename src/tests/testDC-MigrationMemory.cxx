/*
 * testDC.c
 *
 *  Created on: 04/11/2010
 *      Author: pmarchesi
 */
#include <stdio.h>
#include <node/dc/dc.h>
#include <node/dc/imc.h>
#include <unistd.h>
#include <messages/dc-messages.h>
#include "node/daemon/networkManager.h"
#include "ipc/ipcDaemon.h"
#include <util/config.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <limits.h>

#include <ipc/socketBuilder.h>
#include <ipc/socket.h>
#include <util/log_ansi.h>
#include "node/daemon/handshake.h"
#define modulo "testDC-MigrationMemory"
__init_logname(modulo)
int sockListener;
int msgidToNodo;
pthread_t readMsgQueue;

void listenOnMsgQueue();
Lala* test = NULL;

int main(int argc, char** argv) {
	pid_t childpid;
	SocketBuilderClass* builder = new SocketBuilderClass();
	fprintf(stderr, "init \n");
	builder->ip("192.168.0.101");
	builder->puerto("1545");
	fprintf(stderr, "togate \n");
	SocketGateClass* gate = builder->createBind();

	if ((childpid = fork()) < 0) {
		perror("Fork Failed");
		exit(1);
	} else if (childpid != 0) {
		char* arg[5];
		fprintf(stderr, "child \n");
		arg[0] = read_config("./config/daemon.xml", "dc_main.bin");
		arg[1] = "192.168.0.101";
		arg[2] = "1545";
		arg[3] = "0";
		arg[4] = NULL;
		setsid();
		execv(arg[0], arg);
	} else {
		fprintf(stderr, "toacc \n");
		SocketClass<>* sock = gate->acceptConnection();
		if (!server_handshake(sock)) {
			exit(1);
		}
		sockListener = sock->id();
		fprintf(stderr, "topthre \n");

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + 1024);


		pthread_create(&readMsgQueue, NULL, (void* (*)(void*)) listenOnMsgQueue, NULL);
		
    pthread_attr_destroy(&attr);
		fprintf(stderr, "tojoin \n");
		pthread_join(readMsgQueue, NULL);
		fprintf(stderr, "Termino Test\n");
	}

}
int enviar_estado_del_job(Job* send) {
	/*Job* toSend = (Job*) malloc(sizeof(Job));
	 (*toSend) = (*send);
	 addJob(toSend);*/
	return 0;
}
int enviar_estado_de_la_task(Task* send) {
	/*Task* toSend = (Task*) malloc(sizeof(Task));
	 (*toSend) = (*send);
	 addTask(toSend);*/
	return 0;
}
int enviar_estado_de_la_dependencia(DependencyNet* send) {
	/*DependencyNet* toSend = (DependencyNet*) malloc(sizeof(DependencyNet));
	 (*toSend) = (*send);
	 addDepen(toSend);*/
	return 0;
}

int enviar_migracion_completa() {
	/*migracionComplete();*/
}

int enviar_nodo_tarea(NodeWorking* send) {
	/*NodeTask* toSend = (NodeTask*) malloc(sizeof(NodeTask));
	 *toSend = *send;
	 addNodeTask(toSend);*/
}
void se_quiere_migrar() {
	sleep(2);
	insomnio_header header = { "", TYPE_IPC_DC_GET_MEMORY, 0, 0, 0 };
	sprintf(header.id, "%s", "TEST");
	sendMessage(sockListener, &header, NULL, 0);
}
void guardar_memoria_de_dc(void* memo, unsigned int size, int type){
	Lala* tmp = (Lala*) malloc(sizeof(Lala));
	tmp->type = type;
	tmp->mem = memo;
	tmp->size = size;
	tmp->next = test;
	test = tmp;
}
void listenOnMsgQueue() {
	int sock = sockListener;
	int toWhile = 1;
	while (toWhile) {
		fprintf(stderr, "Test: Voy a leer un Header %d\n", sock);
		insomnio_header header;
		int rta = readMessage(sock, &header, sizeof(insomnio_header));
		if (rta == 0) {
			debug(modulo, "Se cerro el socket");
			toWhile = 0;
		}
		fprintf(stderr, "Test:Empiezo a leer mensaje Mensaje (%d) %s  %c\n", rta, header.id, header.payload);
		switch (header.payload) {
		case TYPE_IPC_DC_MIGRATE: {
			se_quiere_migrar();
		}
			break;
		case TYPE_IPC_I_AM_DC: {
			fprintf(stderr, "Se conecto el DC\n");
		}
			break;
		case TYPE_IPC_ADD_JOB: {
			fprintf(stderr, "Llego un Job\n");
			Job* job = (Job*) malloc(sizeof(Job));
			readMessage(sock, job, sizeof(Job));
			//enviar_estado_del_job(job);
			//free(job);
			guardar_memoria_de_dc(job, sizeof(job), header.payload);
		}
			break;
		case TYPE_IPC_ADD_TASK: {
			fprintf(stderr, "Test: Llego una Task\n");
			Task* task = (Task*) malloc(sizeof(Task));
			readMessage(sock, task, sizeof(Task));
			guardar_memoria_de_dc(task, sizeof(Task), header.payload);
		}
			break;
		case TYPE_IPC_ADD_DEP: {
			fprintf(stderr, "Llego una Dependency\n");
			DependencyNet* dep = (DependencyNet*) malloc(sizeof(DependencyNet));
			readMessage(sock, dep, sizeof(DependencyNet));
			guardar_memoria_de_dc(dep, sizeof(DependencyNet), header.payload);
		}
			break;
		case TYPE_IPC_ADD_NODO_TASK: {
			fprintf(stderr, "Llego una Relacion Nodo-Tarea\n");
			NodeWorking* nodeTask = (NodeWorking*) malloc(sizeof(NodeWorking));
			readMessage(sock, nodeTask, sizeof(NodeWorking));
			guardar_memoria_de_dc(nodeTask, sizeof(NodeWorking), header.payload);
		}
			break;
		case TYPE_IPC_MIGRATION_COMPLETE: {
			fprintf(stderr, "Migracion Completa\n");
			enviar_migracion_completa();
		}
			break;
		case TYPE_IPC_NODO_WORKING: {
			fprintf(stderr, "Llego Working\n");
			info_nodo* work = (info_nodo*) malloc(sizeof(info_nodo));
			readMessage(sock, work, sizeof(info_nodo));
		}
			break;
		case TYPE_IPC_TASK_RESULT: {
			fprintf(stderr, "Llego Task Rrsult");
			TaskResult* task = (TaskResult*) malloc(sizeof(TaskResult));
			readMessage(sock, task, sizeof(TaskResult));
		}
			break;
		case TYPE_IPC_NEW_NODO: {
			fprintf(stderr, "Llego un Job");
			info_nodo* newNodo = (info_nodo*) malloc(sizeof(info_nodo));
			readMessage(sock, newNodo, sizeof(info_nodo));
		}
			break;
		case TYPE_IPC_ENDED_NODO: {
			fprintf(stderr, "Llego Ended Nodo");
			info_nodo* nodo = (info_nodo*) malloc(sizeof(info_nodo));
			readMessage(sock, nodo, sizeof(info_nodo));
		}
			break;
		default:
			fprintf(stderr, "Test:ERROR %s.\n", header.id);
			break;
		}
	}
}

