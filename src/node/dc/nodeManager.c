/*
 * nodoManager.c
 *
 *  Created on: 13/10/2010
 *      Author: pmarchesi
 */

#include "imc.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <util/weighted.h>
#include "nodeManager.h"
#include <time.h>
#include <util/config.h>
#include <unistd.h>
#include <limits.h>

#include <util/log_ansi.h>
#define modulo "nodeManager"

Nodes* availableNodes = NULL;
Nodes* workingNodes = NULL;
pthread_t checkWorking;
int wTime = 0;

void printNodeAvailables() {
	Nodes* searchNode = availableNodes;
	debug(modulo, "\tNodos habilitados para ejectutar\n");
	while (searchNode != NULL) {
		debug(modulo, "\t\tNodo %s(%lu) memory(%lu) disk(%lu)\n", searchNode->node->nombre, searchNode->weighted, searchNode->node->memoria, searchNode->node->disco);
		searchNode = searchNode->next;
	}
}
void printNodeWorking() {
	Nodes* searchNode = workingNodes;
	debug(modulo, "\tNodos Ejecutando\n");
	while (searchNode != NULL) {
		debug(modulo, "\t\tNodo %s(%lu)  memory(%lu) disk(%lu) - Tarea %s   memory(%d)  disk(%d) Estado: %s\n", searchNode->node->nombre, searchNode->weighted, searchNode->node->memoria,
				searchNode->node->disco, searchNode->task->task->id, searchNode->task->task->resources.memory, searchNode->task->task->resources.disk, searchNode->task->task->status.actual);
		searchNode = searchNode->next;
	}
}
int removeOfAvailables(Nodes* node) {
	Nodes* searchNode = availableNodes;
	if (availableNodes == NULL)
		return 1;
	if (strcmp(searchNode->node->nombre, node->node->nombre) == 0) {
		availableNodes = availableNodes->next;
		return 0;
	}
	while (searchNode->next != NULL) {
		if (strcmp(searchNode->next->node->nombre, node->node->nombre) == 0) {
			searchNode->next = searchNode->next->next;
			return 0;
		}
		searchNode = searchNode->next;
	}
	return 1;
}
int removeOfWorking(Nodes* node) {
	debug(modulo, "Nodo a quitar en Working %s\n", node->node->nombre);
	if (workingNodes == NULL)
		return 1;

	Nodes* searchNode = workingNodes;
	debug(modulo, "1ro de la lista %s\n", searchNode->node->nombre);
	if (strcmp(searchNode->node->nombre, node->node->nombre) == 0) {
		workingNodes = workingNodes->next;
		debug(modulo, "era 1ro de la lista %s\n", searchNode->node->nombre);
		return 0;
	}
	while (searchNode->next != NULL) {
		debug(modulo, "iterandos la lista %s\n", searchNode->next->node->nombre);
		if (strcmp(searchNode->next->node->nombre, node->node->nombre) == 0) {
			debug(modulo, "lo encontre en la lista %s\n", searchNode->next->node->nombre);
			searchNode->next = searchNode->next->next;
			return 0;
		}
		searchNode = searchNode->next;
	}
	return 1;
}

TasksAvailable* getTaskOfNodo(info_nodo* node) {
	Nodes* searchNode = workingNodes;
	debug(modulo, "Buscar nodo %s\n", node->nombre);
	while (searchNode != NULL) {
		debug(modulo, "Comparar %s\n", searchNode->node->nombre);
		debug(modulo, "Con %s\n", node->nombre);
		if (strcmp(searchNode->node->nombre, node->nombre) == 0) {
			return searchNode->task;
		}
		searchNode = searchNode->next;
	}
	return NULL;
}

void assignTaskToNode(Nodes* node, TasksAvailable* task) {
	if (removeOfAvailables(node)) {
		debug(modulo, "NM->(%d) ERROR!! El nodo(%d) encontrado ahora no se encuentra disponible\n", pthread_self(), node->node->nombre);
		return;
	}
	TasksAvailable* taskQ = (TasksAvailable*) malloc(sizeof(TasksAvailable));
	strcpy(taskQ->id_job, task->id_job);
	taskQ->task = task->task;
	node->task = taskQ;
	node->lastHeartbeat = time(NULL);
	node->next = workingNodes;
	workingNodes = node;

	AssignTask* assign = (AssignTask*) malloc(sizeof(AssignTask));
	assign->task = *(taskQ->task);
	debug(modulo, "Task  %d  %d   %s  %s\n", &assign->task, taskQ->task, assign->task.id, taskQ->task->id);
	strcpy(assign->id_job, taskQ->id_job);
	strcpy(assign->nodo, node->node->nombre);
	debug(modulo, "Envio al IMC la assignacion %s-%s\n", assign->task.id, assign->nodo);
	imc_notify(ASSIGN_TASK, assign);
}

Nodes* nm_searchNodeToTask(Task* task) {
	debug(modulo, "nm_searchNodeToTask Task %s  Primer nodo %s\n", task->id, (availableNodes != NULL ? availableNodes->node->nombre : "null"));
	Nodes* searchNode = availableNodes;
	while (searchNode != NULL) {
		if (task->resources.memory <= searchNode->node->memoria && task->resources.disk <= searchNode->node->disco) {
			debug(modulo, "Nodo encontrado Nodo=%s\n", searchNode->node->nombre);
			return searchNode;
		}
		debug(modulo, "Verificando nodo %s para la tarea %s\n", searchNode->node->nombre, task->id);
		sleep(1);
		searchNode = searchNode->next;
	}
	debug(modulo, "NM->(%d) nm_searchNodeToTask: No se encontro nodo\n", pthread_self());
	return NULL;
}

void nm_cancelTasksOfJob(Job* job) {
	Nodes* nodesResult = NULL;
	Nodes* nodes = workingNodes;
	while (nodes != NULL) {
		if (strcmp(job->id, nodes->task->id_job) == 0) {
			char* nombreNodo = (char*) malloc(15);
			strcpy(nombreNodo, nodes->node->nombre);
			imc_notify(CANCEL_TASK, nombreNodo);
		}
		nodes = nodes->next;
	}
}

Nodes* nm_getAndRemoveNode(const char* task) {
	debug(modulo, "NM->(%d) nm_getAndRemoveNode\n", pthread_self());
	Nodes* searchNode = workingNodes;
	if (workingNodes == NULL) {
		return NULL;
	}
	if (strcmp(searchNode->task->task->id, task) == 0) {
		debug(modulo, "NM->(%d) nm_getAndRemoveNode Era el 1ro\n", pthread_self());
		Nodes* findNode = searchNode;
		workingNodes = searchNode->next;
		findNode->next = NULL;
		return findNode;
	}
	while (searchNode->next != NULL) {

		if (strcmp(searchNode->next->task->task->id, task) == 0) {
			Nodes* findNode = searchNode->next;
			searchNode->next = findNode->next;
			findNode->next = NULL;
			return findNode;
		}
		searchNode = searchNode->next;
	}
	return NULL;
}

void nm_addNodeOrderly(Nodes* node) {
	debug(modulo, "NM->(%d) nm_addNodeOrderly\n", pthread_self());
	if ((availableNodes == NULL) || (availableNodes->weighted < node->weighted)) {
		debug(modulo, "NM->(%d) nm_addNodeOrderly-1\n", pthread_self());
		node->next = availableNodes;
		availableNodes = node;
	} else {
		debug(modulo, "NM->(%d) nm_addNodeOrderly-2\n", pthread_self());
		Nodes* searchPosition = availableNodes;
		debug(modulo, "NM->(%d) nm_addNodeOrderly busco\n", pthread_self());
		while ((searchPosition->next != NULL) && ((searchPosition->next)->weighted > node->weighted)) {
			searchPosition = searchPosition->next;
		}
		debug(modulo, "NM->(%d) nm_addNodeOrderly encuentro\n", pthread_self());
		if (searchPosition->next != NULL) {
			debug(modulo, "NM->(%d) nm_addNodeOrderly genero continuacion\n", pthread_self());
			node->next = searchPosition->next;
		}
		debug(modulo, "NM->(%d) nm_addNodeOrderly agrego\n", pthread_self());
		searchPosition->next = node;
	}
}

void nm_newNodo(void* arg) {
        info(modulo,"Nuevo nodo %s\n",((info_nodo*) arg)->nombre);
	debug(modulo, "NM->(%d) nm_newNodo\n", pthread_self());
	Nodes* searchNodo = getNodesAvailables();
	while (searchNodo != NULL && strcmp(searchNodo->node->nombre, ((info_nodo*) arg)->nombre)) {
		searchNodo = searchNodo->next;
	}
	if (searchNodo == NULL) {
		searchNodo = getNodesWorking();
		while (searchNodo != NULL && strcmp(searchNodo->node->nombre, ((info_nodo*) arg)->nombre)) {
			searchNodo = searchNodo->next;
		}
	}
	if (searchNodo != NULL) {
		searchNodo->node = ((info_nodo*) arg);
		searchNodo->weighted = getWeightedNodo(*searchNodo->node);
	} else {
		Nodes* node = (Nodes*) malloc(sizeof(Nodes));
		node->node = (info_nodo*) arg;
		node->weighted = getWeightedNodo(*node->node);
		node->task = NULL;
		node->next = NULL;
		nm_addNodeOrderly(node);
	}
	printNodeAvailables();
	printNodeWorking();
}

void nm_nodeEnded(info_nodo* arg) {
    info(modulo, "Se cayo un nodo");
	debug(modulo, "NM->(%d) nm_nodeEnded %s\n", pthread_self(), arg->nombre);
	Nodes* node = (Nodes*) malloc(sizeof(Nodes));
	node->node = (info_nodo*) arg;
	if (removeOfAvailables(node) == 1) { /*No lo encontro*/
		if (removeOfWorking(node) == 1) { /*No lo encontro*/
			debug(modulo, "No conocia el nodo %s\n", node->node->nombre);
		} else {
                    info(modulo, "Sacando %s estaba trabajando\n", node->node->nombre);
                }
	} else {
            info(modulo, "Sacando %d de disponibles\n",node->node->nombre);
        }
	printNodeAvailables();
	printNodeWorking();
}
int nm_isAssigned(Task* task) {
	debug(modulo, "NM->(%d) nm_isAssigned \n", pthread_self());
	Nodes* nodes = workingNodes;
	if (nodes == NULL) {
		return 0;
	}
	while (nodes != NULL) {
		if (strcmp(nodes->task->task->id, task->id) == 0) {
			return 1;
		}
		nodes = nodes->next;
	}
	return 0;
}

void nm_newTasks(void* args) {
	debug(modulo, "NM->(%d) nm_newTasks\n", pthread_self());
	TasksAvailable* tasks = (TasksAvailable*) args;
	while (tasks != NULL) {
		if (!nm_isAssigned(tasks->task)) {
			debug(modulo, "NM->(%d) nm_newTasks Hay tareas\n", pthread_self());
			Nodes* node = nm_searchNodeToTask(tasks->task);
			if (node != NULL) {
                            info(modulo, "Asignar %s al nodo %s\n", tasks->task->id, node->node->nombre);
				debug(modulo, "NM->(%d) nm_newTasks encontre Nodo %s\n", pthread_self(), node->node->nombre);
				assignTaskToNode(node, tasks);
			} else {
                            info(modulo, "No se encontro nodo para %s\n", tasks->task->id);
                        }
		}
		debug(modulo, "Siguiente tarea %d\n", tasks->next);
		tasks = tasks->next;
	}
	printNodeAvailables();
	printNodeWorking();
}

void nm_taskEnded(void* task) {
    TaskResult* _task = (TaskResult*) task;
    info(modulo, "Tarea terminada %s estado %s \n", _task->id_task, TASK_NAME(_task->status) );
	debug(modulo, "NM->(%d) nm_taskEnded %s\n", pthread_self(), _task->id_task);
	Nodes* node = nm_getAndRemoveNode(_task->id_task);
	if (node == NULL) {
		debug(modulo, "NM->(%d) ERROR!! La tarea(%s) no se encontraba ejecutandose\n", pthread_self(), _task->id_task);
	} else {
		node->task = NULL;
		nm_addNodeOrderly(node);
	}
	printNodeAvailables();
	printNodeWorking();
}

void nm_jobEnded(void* job) {
    info(modulo,"Job %s terminado, cancelando tareas\n",((Job*)job)->id);
	debug(modulo, "NM->(%d) nm_jobEnded\n", pthread_self());
	nm_cancelTasksOfJob((Job*) job);
	printNodeAvailables();
	printNodeWorking();
}

void nm_failTaskNode(info_nodo* node) {
	debug(modulo, "NM->(%d) nm_failTaskNode\n", pthread_self());
	TasksAvailable* task = getTaskOfNodo(node);
	if (task == NULL) {
		error(modulo, "El nodo %s no tenia tarea asignada\n", node->nombre);
		return;
	}
	imc_notify(CANCEL_TASK, node->nombre);
	TaskResult* result = (TaskResult*) malloc(sizeof(TaskResult));
	strcpy(result->id_job, task->id_job);
	strcpy(result->id_task, task->task->id);
	result->status = TASK_FAIL;
	strcpy(result->value, task->task->status.actual);
	imc_notify(TASK_ENDED, result);
}
Nodes* nm_getNodo(char* nodoNombre) {
	Nodes* searchNode = availableNodes;
	while (searchNode != NULL) {
		if (strcmp(searchNode->node->nombre, nodoNombre) == 0) {
			return searchNode;
		}
		searchNode = searchNode->next;
	}
	searchNode = workingNodes;
	while (searchNode != NULL) {
		if (strcmp(searchNode->node->nombre, nodoNombre) == 0) {
			return searchNode;
		}
		searchNode = searchNode->next;
	}
	return NULL;
}

void nm_nodeWorking(info_nodo* nodo) {
	debug(modulo, "Working de %s\n",nodo->nombre);

        Nodes* nodes = nm_getNodo(nodo->nombre);
	if (nodes == NULL) {
		debug(modulo, "El nodo %s esta respondiendo working y no lo tengo trabajando\n", nodo->nombre);
	} else {
		debug(modulo, "Recibi un working del nodo %s\n", nodo->nombre);
		nodes->lastHeartbeat = time(NULL);
	}
}

int keepCheckWorking = 1;

void nm_threadCheckWorking() {
	debug(modulo, "init Check Working\n");
	while (keepCheckWorking) {
		sleep(2);
		debug(modulo, "Check Working...\n");
		Nodes* node = workingNodes;
		while (node != NULL) {
			float timeTranscured = (float) difftime(time(NULL), node->lastHeartbeat);
			if (timeTranscured > wTime) {
				imc_notify(FAIL_TASK_NODE, node->node);
				info(modulo, "El nodo %s no responde working, le saque la tarea\n", node->node->nombre);
			}
			node = node->next;
		}
	}
	debug(modulo, "end Check Working\n");
	pthread_exit(NULL);
}

void nm_getInfo(DCState* state) {
	debug(modulo, "GET-INFO Adjuntando mi informacion, le paso la bola al IMC\n");
	int count = 0;
	Nodes* tmp = workingNodes;
	while (tmp) {
		count++;
		tmp = tmp->next;
	}
	state->cant_running_task = count;
}

void initNM() {
	debug(modulo, "Init NodoManager\n");
	wTime = atoi(read_config("config/dc/nodeManager.xml", "working_time"));
	debug(modulo, "wTime %d\n", wTime);
	imc_suscribe(NEW_NODE, (Function*) &nm_newNodo);
	imc_suscribe(NODE_ENDED, (Function*) &nm_nodeEnded);
	imc_suscribe(NODE_WORKING, (Function*) &nm_nodeWorking);
	imc_suscribe(TASK_ENDED, (Function*) &nm_taskEnded);
	imc_suscribe(JOBS_ENDED, (Function*) &nm_jobEnded);
	imc_suscribe(AVAILABLE_TASKS, (Function*) &nm_newTasks);
	imc_suscribe(FAIL_TASK_NODE, (Function*) &nm_failTaskNode);
}
void startNM() {
	debug(modulo, "Levanto Thread de Working\n");

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + 1024);

	pthread_create(&checkWorking, NULL, (void* (*)(void*)) nm_threadCheckWorking, NULL);
	
    pthread_attr_destroy(&attr);
	debug(modulo, "Levantando (Thread de Working)\n");
}
void stopNM() {
	/*
	 debug(modulo, "Stop thread check working\n");
	 */
	keepCheckWorking = 0;
	/*
	 debug(modulo, "thread check working detenido\n");
	 */
}
Nodes* getNodesAvailables() {
	return availableNodes;
}

Nodes* getNodesWorking() {
	return workingNodes;
}

void addNodoTaskNM(Nodes* node) {
	debug(modulo, "node\n");
	node->weighted = getWeightedNodo(*node->node);
	debug(modulo, "node1 %d\n", workingNodes);
	node->next = workingNodes;
	debug(modulo, "node2 %d\n", node->next);
	workingNodes = node;
	debug(modulo, "node3 %d\n", workingNodes);
}
