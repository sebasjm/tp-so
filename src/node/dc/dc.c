/*
 * dc.c
 *
 *  Created on: 13/10/2010
 *      Author: pmarchesi
 */
#include "dc.h"
#include "imc.h"
#include "nodeManager.h"
#include "webManager.h"
#include "taskManager.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <messages/daemon-daemon.h>
#include <ipc/ipcDaemon.h>
#include "node/daemon/networkManager.h"
#include <pthread.h>
#include <unistd.h>
#include "node/daemon/common.h"
#include <limits.h>


#include <util/log_ansi.h>
#define modulo "dc"

Job* newJob = NULL;
int numTasks = 0;
int numDepen = 0;
DependeciesNet* depens = NULL;
int sockNodo = 0;
pthread_t readMsgQueue;

void sigusrToMigration(int sig);
void listenOnMsgQueue();
void assignTask(AssignTask* assign);
void cancelTask(char* nodo);
void getInfoComplete(void* data);
void migrate_dc();

insomnio_header* makeHeader(char* id, int type);

void initDC(char** argv) {
	debug(modulo, "iniciando IMC\n");
	initIMC();
	debug(modulo, "iniciando WM\n");
	initWM();
	debug(modulo, "iniciando NM\n");
	initNM();
	debug(modulo, "iniciando TM\n");
	initTM();

	imc_suscribe(ASSIGN_TASK, (Function*) &assignTask);
	imc_suscribe(CANCEL_TASK, (Function*) &cancelTask);
	imc_suscribe(GET_INFO_COMPLETE, (Function*) &getInfoComplete);

	sockNodo = conectar_a(argv[1], argv[2]);
	if (sockNodo <= 0) {
		error(modulo, "Error al intentar crear el conexion con el nodo");
		exit(1);
	}
	sendMessage(sockNodo, makeHeader("Hola soy DC", TYPE_IPC_I_AM_DC), NULL, 0);

        set_dc_ip_port(argv[1], argv[2]);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + 1024);

	pthread_create(&readMsgQueue, NULL, (void* (*)(void*)) listenOnMsgQueue, NULL);
	
    pthread_attr_destroy(&attr);
}
void startDC(int migration) {
	if (migration == 0) {
		debug(modulo, "arrancando IMC\n");
		startIMC();
		debug(modulo, "arrancando WM\n");
		pthread_t wm = startWM();
		debug(modulo, "arrancando NM\n");
		startNM();
		debug(modulo, "listo arrancando DC, ahora configuro todo lo que tenga q configurar\n");
		debug(modulo, "\tNada\n");

		debug(modulo, " esperando a que termine el WM \n");
		void* result;
		pthread_join(readMsgQueue, &result);
	} else {
		sendMessage(sockNodo, makeHeader("New DC", TYPE_IPC_DC_GET_MEMORY), NULL, 0);
		info(modulo, "esperando memoria el otro DC\n");
		void* result;
		pthread_join(readMsgQueue, &result);
		info(modulo, "Miruo el readMsgQueue\n");
		sleep(7);info(modulo, "Miruo el readMsgQueue 7\n");
	}
}

void stopDC() {
/*
	debug(modulo, "Stop iniciado!\n");
*/
	stopWM();
	stopIMC();
	stopNM();
/*
	debug(modulo, "Stop complete!\n");
*/
}

void sigusrToMigration(int sig) {
	debug(modulo, "Llego senal %d (SIGUSR1=%d)\n", sig, SIGUSR1);
	stopDC();
	migrate_dc();
}
void migrate_dc() {
/*
	debug(modulo, "Migracion iniciada\n");
*/
	newDC();
/*
	debug(modulo, "Migracion completa\n");
*/
	exit(0);
}
/* Envio de Memoria de DC*/
long countTask(Tasks* tasks) {
	long count = 0;

	while (tasks != NULL) {
		count++;
		tasks = tasks->next;
	}
	return count;
}
insomnio_header* makeHeader(char* id, int type) {
	insomnio_header* header = (insomnio_header*) malloc(sizeof(insomnio_header));
	strcpy(header->id, id);
	header->payload = type;
	header->ttl = 9;
	header->hops = 0;
	header->length = 0;
	return header;
}
void assignTask(AssignTask* assign) {
	info(modulo, "Enviando asignacion %s en %s\n",assign->task.id ,assign->nodo);
	sendMessage(sockNodo, makeHeader("IDJ", TYPE_IPC_ASSIGN_TASK), assign, sizeof(AssignTask));
}
void cancelTask(char* nodo) {
	info (modulo, "Enviando cancelacion de %s\n", nodo);
	sendMessage(sockNodo, makeHeader("IDJ", TYPE_IPC_CANCEL_TASK), nodo, sizeof(nombre_nodo));
}
void getInfoComplete(void* data) {
	info (modulo, "Get Info Completed\n");
        sendMessage(sockNodo, makeHeader("INFO", TYPE_IPC_INFO_DC_COMPLETE), data, sizeof(DCState) );
}
void newDC() {
/*
	info(modulo, "Comienzo la migracion\n");
*/
	Job* jobNet = getWorkingJob();
	if (jobNet != NULL) {

		Tasks* tasks = jobNet->tasks;
		jobNet->tasks = (Tasks*) countTask(tasks);
/*
		debug(modulo, "Envio Job %s\n", jobNet->id);
*/
		sendMessage(sockNodo, makeHeader("IDJ", TYPE_IPC_ADD_JOB), jobNet, sizeof(Job));
		while (tasks != NULL) {
			Tasks* depen = tasks->task->dependencies;
			tasks->task->dependencies = (Tasks*) countTask(depen);
/*
			debug(modulo, "Envio Task %s\n", tasks->task->id);
*/
			sendMessage(sockNodo, makeHeader("IDT", TYPE_IPC_ADD_TASK), tasks->task, sizeof(Task));
			while (depen != NULL) {
				DependencyNet depenNet;
				strcpy(depenNet.idTask, tasks->task->id);
				strcpy(depenNet.idDepen, depen->task->id);
/*
				debug(modulo, "Envio Dependency %s - %s\n", depenNet.idTask, depenNet.idDepen);
*/
				sendMessage(sockNodo, makeHeader("IDD", TYPE_IPC_ADD_DEP), &depenNet, sizeof(DependencyNet));
				depen = depen->next;
			}
			tasks = tasks->next;
		}

		Nodes* working = getNodesWorking();
		while (working != NULL) {
			NodeWorking* nodeTask = (NodeWorking*) malloc(sizeof(NodeWorking));
			nodeTask->nodo = *(working->node);
			nodeTask->lastHeartbeat = working->lastHeartbeat;
			strcpy(nodeTask->task, working->task->task->id);
/*
			debug(modulo, "Envio NodoTask %s - %s\n", nodeTask->nodo.nombre, nodeTask->task);
*/
			sendMessage(sockNodo, makeHeader("IDNT", TYPE_IPC_ADD_NODO_TASK), nodeTask, sizeof(NodeWorking));
			working = working->next;
		}
	}
	sendMessage(sockNodo, makeHeader("ID_COM", TYPE_IPC_MIGRATION_COMPLETE), NULL, 0);
/*
	debug(modulo, "Envio Migracion Completa!\n");
*/
}
/* END Envio de Memoria de DC*/

/* Recibo de Memoria de DC*/
void addJob(Job* jobReceive) {
	debug(modulo, "Job %s  Target(%s, %d, %d) Cantidad de Tareas %d\n", jobReceive->id, jobReceive->target.hash, jobReceive->target.length, jobReceive->target.method, jobReceive->tasks);
	numTasks = (long) jobReceive->tasks;
	jobReceive->tasks = NULL;
	newJob = jobReceive;
}
void addTask(Task* task) {
	debug(modulo, "Tasks(%d) %s  Desc %s\n", task, task->id, task->description);
	debug(modulo, "\tResources  (%d, %lu, %lu)\n", task->resources.cpu, task->resources.disk, task->resources.memory);
	debug(modulo, "\tStatus (%d, %d, %d, %s, %s, %s, %s)\n", task->status.length, task->status.method, task->status.status, task->status.hash, task->status.start, task->status.actual,
			task->status.end);
	debug(modulo, "\tCantidad de Dependencias %d\n", task->dependencies);
	if (numTasks <= 0) {
		debug(modulo, "DC.addTask  ERROR GRAVE: Tarea no esperada %s\n", task->id);
		exit(3);
	}
	Tasks* tasks = (Tasks*) malloc(sizeof(Tasks));
	tasks->task = task;
	numDepen = (long) task->dependencies;
	task->dependencies = NULL;
	tasks->next = newJob->tasks;
	newJob->tasks = tasks;
	numTasks--;
}
void addDepen(DependencyNet* depen) {
	debug(modulo, "Dependencia de %s  con  %s\n", depen->idTask, depen->idDepen);
	if (numDepen <= 0) {
		debug(modulo, "DC.addDepen  ERROR GRAVE: Dependencia no esperada %s-%s\n", depen->idTask, depen->idDepen);
		exit(3);
	}
	DependeciesNet* depenNet = (DependeciesNet*) malloc(sizeof(DependeciesNet));
	depenNet->depen = *depen;
	depenNet->next = depens;
	depens = depenNet;
	numDepen--;
}
DependeciesNet* getDependency(char* idTask) {
	DependeciesNet* depen = depens;
	if (depens == NULL) {
		return NULL;
	}
	if (strcmp(depen->depen.idTask, idTask) == 0) {
		depens = depen->next;
		return depen;
	}

	while (depen->next != NULL) {
		if (strcmp(depen->next->depen.idTask, idTask) == 0) {
			DependeciesNet* result = depen;
			depen->next = depen->next->next;
			return result;
		}
		depen = depen->next;
	}
	return NULL;
}

Task* searchTask(char* idTask) {
	Tasks* tasks = newJob->tasks;
	while (tasks != NULL && strcmp(tasks->task->id, idTask) != 0) {
		tasks = tasks->next;
	}
	if (tasks == NULL) {
		debug(modulo, "DC.searchTask  ERROR GRAVE: No se encontro la tarea %s\n", idTask);
		exit(4);
	}
	return tasks->task;
}

void addNodeTask(NodeWorking* nodeTask) {
	Nodes* node = (Nodes*) malloc(sizeof(Nodes));
	debug (modulo, "NW 1\n");
	node->node = &nodeTask->nodo;
	debug (modulo, "NW 2\n");
	node->lastHeartbeat = nodeTask->lastHeartbeat;
	debug (modulo, "NW 3\n");
	node->next = NULL;
	debug (modulo, "NW 4 %s\n", nodeTask->task);
	node->task = (TasksAvailable*) malloc(sizeof(TasksAvailable));
	node->task->task = searchTask(nodeTask->task);
	debug (modulo, "NW 5\n");
	node->task->next = NULL;
	debug (modulo, "NW 6\n");
	strcpy(node->task->id_job, newJob->id);
	debug (modulo, "NW 7 %s\n", node->task->id_job);
	addNodoTaskNM(node);
	/*nodes->*/
}

void clean_mem() {
	newJob = NULL;
	numTasks = 0;
	numDepen = 0;
	depens = NULL;
}

void migracionComplete() {
    info(modulo,"Migracion completa\n");
        if (newJob != NULL) {
            info(modulo, "El recepcion del job se completo, se procede a armarlo\n");
            Tasks* it = newJob->tasks;
            while (it != NULL) {
                debug(modulo, "%s\n", it->task->id);
                DependeciesNet* dep = getDependency(it->task->id);
                while (dep != NULL) {
                    Dependencies* tasksDep = (Tasks*) malloc(sizeof (Tasks*));
                    debug(modulo, "%s - %s\n", dep->depen.idTask, dep->depen.idDepen);
                    tasksDep->task = searchTask(dep->depen.idDepen);
                    debug(modulo, "Dep %s\n", tasksDep->task->id);
                    tasksDep->next = it->task->dependencies;
                    it->task->dependencies = tasksDep;
                    dep = getDependency(it->task->id);
                }
                it = it->next;
            }
            setJobWM(newJob);
            setJobTM(newJob);
        } else {
            info(modulo, "No hay job activo para migrar\n");
        }
	clean_mem();
	debug(modulo, "Se genero la migracion sin errores\n");
	startDC(0);

}
/*END Recibo de Memoria de DC*/

void listenOnMsgQueue() {
	int toWhile = 1;
	while (toWhile) {
		debug(modulo, "DC: Voy a leer un Header %d\n", sockNodo);
		insomnio_header header;
		int rta = readMessage(sockNodo, &header, sizeof(insomnio_header));
		if (rta == 0) {
			debug(modulo, "Se cerro el socket\n");
                        close(sockNodo);
			toWhile = 0;
			continue;
		} else if (rta == -1) {
			error(modulo, "Error al intentar leer un header\n");
			toWhile = 0;
			continue;
		}
		debug(modulo, "DC: Empiezo a leer mensaje Mensaje (Leido %d) %s - %d\n", rta, header.id, header.payload);
		switch (header.payload) {
		case TYPE_IPC_DC_GET_MEMORY: {
			debug(modulo, "DC Migrate\n");
			migrate_dc();
			debug(modulo, "Ya se migro\n");
		}
			break;
		case TYPE_IPC_DC_MIGRATE_ERROR: {
			debug(modulo, "Error al intentar Migrarse\n");
			stopDC();
			exit(0);
		}
			break;
		case TYPE_IPC_ADD_JOB: {
			debug(modulo, "Llego un Job\n");
			Job* job = (Job*) malloc(sizeof(Job));
			readMessage(sockNodo, job, sizeof(Job));
			addJob(job);
		}
			break;
		case TYPE_IPC_ADD_TASK: {
			debug(modulo, "Llego una Task\n");
			Task* task = (Task*) malloc(sizeof(Task));
			readMessage(sockNodo, task, sizeof(Task));
			addTask(task);
		}
			break;
		case TYPE_IPC_ADD_DEP: {
			debug(modulo, "Llego una Dependency\n");
			DependencyNet* dep = (DependencyNet*) malloc(sizeof(DependencyNet));
			readMessage(sockNodo, dep, sizeof(DependencyNet));
			addDepen(dep);
		}
			break;
		case TYPE_IPC_ADD_NODO_TASK: {
			debug(modulo, "Llego una Relacion Nodo-Tarea\n");
			NodeWorking* nodeTask = (NodeWorking*) malloc(sizeof(NodeWorking));
			readMessage(sockNodo, nodeTask, sizeof(NodeWorking));
			addNodeTask(nodeTask);
		}
			break;
		case TYPE_IPC_MIGRATION_COMPLETE: {
			debug(modulo, "Migracion Completa\n");
			migracionComplete();
		}
			break;
		case TYPE_IPC_INFO_DC: {
			debug(modulo, "Me piden que informe mi estado, delegando\n");
                        DCState* state = (DCState*) malloc( sizeof(DCState) );
                        state->conexion_con_interfaz = header.ttl;
			imc_notify(GET_INFO_DC, state);
		}
			break;
		case TYPE_IPC_NODO_WORKING: {
			debug(modulo, "Llego Working\n");
			info_nodo* work = (info_nodo*) malloc(sizeof(info_nodo));
			readMessage(sockNodo, work, sizeof(info_nodo));
			debug(modulo, "\t de %s\n", work->nombre);
			imc_notify(NODE_WORKING, work);
		}
			break;
		case TYPE_IPC_TASK_RESULT: {
			debug(modulo, "Llego Task Result\n");
			TaskResult* task = (TaskResult*) malloc(sizeof(TaskResult));
			readMessage(sockNodo, task, sizeof(TaskResult));
			imc_notify(TASK_ENDED, task);
		}
			break;
		case TYPE_IPC_NEW_NODO: {
			debug(modulo, "Llego un Nodo neuvo\n");
			info_nodo* newNodo = (info_nodo*) malloc(sizeof(info_nodo));
			readMessage(sockNodo, newNodo, sizeof(info_nodo));
			imc_notify(NEW_NODE, newNodo);
		}
			break;
		case TYPE_IPC_ENDED_NODO: {
			debug(modulo, "Llego Ended Nodo\n");
			info_nodo* nodo = (info_nodo*) malloc(sizeof(info_nodo));
			readMessage(sockNodo, nodo, sizeof(info_nodo));
			imc_notify(NODE_ENDED, nodo);
		}
			break;
		default:
			debug(modulo, "DC: ERROR %s - %d.\n", header.id, header.payload);
			break;
		}
	}
	info(modulo, "Paro el DC y lo cierro");
	stopDC();
        exit(0);
}
