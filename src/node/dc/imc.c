/*
 * imc.c
 *
 *  Created on: 30/10/2010
 *      Author: pmarchesi
 */
#include "imc.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <limits.h>

#include <unistd.h>
Element* queue_newJob = NULL;
Element* queue_taskEnded = NULL;
Element* queue_newNode = NULL;
Element* queue_jobsEnded = NULL;
Element* queue_availableNode = NULL;
Element* queue_failTaskNode = NULL;
Element* queue_migrate = NULL;
Element* queue_addJob = NULL;
Element* queue_addTask = NULL;
Element* queue_addDep = NULL;
Element* queue_addNodeTask = NULL;
Element* queue_migrationComplete = NULL;
Element* queue_nodeWorking = NULL;
Element* queue_nodeEnded = NULL;
Element* queue_assignTask = NULL;
Element* queue_cancelTask = NULL;
Element* queue_getInfoDC = NULL;

pthread_t thread_run;
Message* msgQueue = NULL;
pthread_mutex_t mutexMsgQueue;
pthread_cond_t enqueueMsg;

void* tasksToExecute = NULL;

#include <util/log_ansi.h>
#define modulo "imc"

void initIMC() {
	pthread_mutex_init(&mutexMsgQueue, NULL);
	pthread_cond_init(&enqueueMsg, NULL);
}

void addElement(Function* element, Element** elements);
void sendAll(void* arg, Element* elements);

void suscribe_newJob(Function* function) {
	addElement(function, &queue_newJob);
}

void suscribe_taskEnded(Function* function) {
	addElement(function, &queue_taskEnded);
}

void suscribe_newNode(Function* function) {
	addElement(function, &queue_newNode);
}

void suscribe_jobsEnded(Function* function) {
	addElement(function, &queue_jobsEnded);
}

void suscribe_availableTasks(Function* function) {
	addElement(function, &queue_availableNode);
}

void suscribe_failTaskNode(Function* function) {
	addElement(function, &queue_failTaskNode);
}
void suscribe_migrate(Function* function) {
	addElement(function, &queue_migrate);
}
void suscribe_addJob(Function* function) {
	addElement(function, &queue_addJob);
}
void suscribe_addTask(Function* function) {
	addElement(function, &queue_addTask);
}
void suscribe_addDep(Function* function) {
	addElement(function, &queue_addDep);
}
void suscribe_addNodoTask(Function* function) {
	addElement(function, &queue_addNodeTask);
}
void suscribe_migrationComplete(Function* function) {
	addElement(function, &queue_migrationComplete);
}
void suscribe_nodoWorking(Function* function) {
	addElement(function, &queue_nodeWorking);
}
void suscribe_nodoEnded(Function* function) {
	addElement(function, &queue_nodeEnded);
}
void suscribe_assingTask(Function* function) {
	addElement(function, &queue_assignTask);
}
void suscribe_cancelTask(Function* function) {
	addElement(function, &queue_cancelTask);
}
void suscribe_getInfoDC(Function* function) {
	addElement(function, &queue_getInfoDC);
}

void imc_suscribe(int type, Function* function) {
	debug(modulo, "IMC subcribe: type %d \n", type);
	switch (type) {
	case NEW_JOB:
		suscribe_newJob(function);
		break;
	case TASK_ENDED:
		suscribe_taskEnded(function);
		break;
	case NEW_NODE:
		suscribe_newNode(function);
		break;
	case JOBS_ENDED:
		suscribe_jobsEnded(function);
		break;
	case AVAILABLE_TASKS:
		suscribe_availableTasks(function);
		break;
	case FAIL_TASK_NODE:
		suscribe_failTaskNode(function);
		break;
	case MIGRATE:
		suscribe_migrate(function);
		break;
	case ADD_JOB:
		suscribe_addJob(function);
		break;
	case ADD_TASK:
		suscribe_addTask(function);
		break;
	case ADD_DEP:
		suscribe_addDep(function);
		break;
	case ADD_NODE_TASK:
		suscribe_addNodoTask(function);
		break;
	case MIGRATION_COMPLETED:
		suscribe_migrationComplete(function);
		break;
	case NODE_WORKING:
		suscribe_nodoWorking(function);
		break;
	case NODE_ENDED:
		suscribe_nodoEnded(function);
		break;
	case GET_INFO_COMPLETE:
		suscribe_getInfoDC(function);
		break;
	case ASSIGN_TASK:
		suscribe_assingTask(function);
		break;
	case CANCEL_TASK:
		suscribe_cancelTask(function);
		break;
	default:
		debug(modulo, "IMC->(%d) subcribe: message invalid (%d)\n", pthread_self(), type);
	}
}

Message* getNextMessage() {
	pthread_mutex_lock(&mutexMsgQueue);

	Message* nextMessage = msgQueue;
	int findMsg = 0;
	while (findMsg == 0) {
		if (nextMessage == NULL) {
			findMsg = 1;
		} else {
			if (nextMessage->type == AVAILABLE_TASKS) {
				if (tasksToExecute != NULL) {
					nextMessage->arg = tasksToExecute;
					tasksToExecute = NULL;
					debug(modulo, "IMC->(%d) enviadas (%d)\n", pthread_self(), nextMessage->arg);
					findMsg = 1;
				} else {
					nextMessage = nextMessage->next;
				}
			} else {
				findMsg = 1;
			}
		}
	}
	if (nextMessage == NULL) {
		pthread_cond_wait(&enqueueMsg, &mutexMsgQueue);
		if (msgQueue == NULL) {
			debug(modulo, "Luego del wait no hubo mensajes, me estan matando\n");
			return NULL;
		}
		nextMessage = msgQueue;
		if (nextMessage->type == AVAILABLE_TASKS) {
			tasksToExecute = NULL;
			debug(modulo, "TAREAS!!!!!!!!----- enviadas (%d)   encoladas (%d)\n", nextMessage->arg, tasksToExecute);
		}
	}
	msgQueue = nextMessage->next;
	nextMessage->next = NULL; /* No es necesario pero para quedar tranquilos*/
	pthread_mutex_unlock(&mutexMsgQueue);
	return nextMessage;

}
void addMessage(Message* msg) {
	pthread_mutex_lock(&mutexMsgQueue);
	debug(modulo, "IMC->(%d) addMessage: type %d\n", pthread_self(), msg->type);
	if (msgQueue == NULL) {
		msgQueue = msg;
	} else {
		Message* nextMsg = msgQueue;
		while (nextMsg->next != NULL) {
			nextMsg = nextMsg->next;
		}
		nextMsg->next = msg;
	}
	if (msg->type == AVAILABLE_TASKS) {
		tasksToExecute = msg->arg;
	}
	pthread_cond_signal(&enqueueMsg);
	pthread_mutex_unlock(&mutexMsgQueue);
}

void imc_notify(int type, void* arg) {
	debug(modulo, "IMC->(%d) enqueue message: type %d\n", pthread_self(), type);
	Message* msg = (Message*) malloc(sizeof(Message));
	msg->type = type;
	msg->arg = arg;
	msg->next = NULL;
	debug(modulo, "Voy a encolar el mensaje\n");
	addMessage(msg);
	debug(modulo, "Mensaje encolado\n");
}

void addElement(Function* function, Element** elements) {
	Element* element = (Element*) malloc(sizeof(Element));
	element->element = (Function) function;
	element->next = *elements;
	(*elements) = element;
}

void sendAll(void* arg, Element* elements) {
	Element* point = elements;
	while (point != NULL) {
		debug(modulo, "IMC->(%d) send message to function %d\n", pthread_self(), point->element);
		(point->element)(arg);
		point = point->next;
	}
}

void notify_newJob(void* job) {
	sendAll(job, queue_newJob);
}
void notify_taskEnded(void* task) {
	sendAll(task, queue_taskEnded);
}
void notify_newNode(void* task) {
	sendAll(task, queue_newNode);
}
void notify_jobsEnded(void* job) {
	sendAll(job, queue_jobsEnded);
}
void notify_availableTasks(void* node) {
	sendAll(node, queue_availableNode);
}
void notify_failTaskNode(void* node) {
	sendAll(node, queue_failTaskNode);
}
void notify_migrate(void* arg) {
	sendAll(arg, queue_migrate);
}
void notify_addJob(void* arg) {
	sendAll(arg, queue_addJob);
}
void notify_addTask(void* arg) {
	sendAll(arg, queue_addTask);
}
void notify_addDep(void* arg) {
	sendAll(arg, queue_addDep);
}
void notify_addNodoTask(void* arg) {
	sendAll(arg, queue_addNodeTask);
}
void notify_migrationComplete(void* arg) {
	sendAll(arg, queue_migrationComplete);
}
void notify_nodoWorking(void* arg) {
	sendAll(arg, queue_nodeWorking);
}
void notify_nodoEnded(void* arg) {
	sendAll(arg, queue_nodeEnded);
}
void notify_assingTask(void* arg) {
	sendAll(arg, queue_assignTask);
}
void notify_cancelTask(void* arg) {
	sendAll(arg, queue_cancelTask);
}
void notify_getInfoDC(void* arg) {
	sendAll(arg, queue_getInfoDC);
}

void run() {
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	int toWhile = 1;
	while (toWhile) {
		debug(modulo, "IMC->(%d) run: Check queue\n", pthread_self());
		Message* msg = getNextMessage();
		debug(modulo, "IMC->(%d) run: New message enqueue (type %d)\n", pthread_self(), msg->type);
		if (msg == NULL) {
			toWhile = 0;
			continue;
		}
		switch (msg->type) {
		case NEW_JOB:
			notify_newJob(msg->arg);
			continue;
			break;
		case TASK_ENDED:
			notify_taskEnded(msg->arg);
			break;
		case NEW_NODE:
			notify_newNode(msg->arg);
			continue;
			break;
		case JOBS_ENDED:
			notify_jobsEnded(msg->arg);
			break;
		case AVAILABLE_TASKS:
			notify_availableTasks(msg->arg);
			break;
		case FAIL_TASK_NODE:
			notify_failTaskNode(msg->arg);
			continue;
			break;
		case MIGRATE:
			notify_migrate(msg->arg);
			break;
		case ADD_JOB:
			notify_addJob(msg->arg);
			break;
		case ADD_TASK:
			notify_addTask(msg->arg);
			break;
		case ADD_DEP:
			notify_addDep(msg->arg);
			break;
		case ADD_NODE_TASK:
			notify_addNodoTask(msg->arg);
			break;
		case MIGRATION_COMPLETED:
			notify_migrationComplete(msg->arg);
			break;
		case NODE_WORKING:
			notify_nodoWorking(msg->arg);
			break;
		case NODE_ENDED:
			notify_nodoEnded(msg->arg);
			break;
		case ASSIGN_TASK:
			notify_assingTask(msg->arg);
			continue;
			break;
		case GET_INFO_DC: {
			/*el orden es importante*/
			DCState* state = (DCState*) msg->arg;
			nm_getInfo(state);
			tm_getInfo(state);
			wm_getInfo(state);
			notify_getInfoDC(msg->arg);
		}
			break;
		case CANCEL_TASK:
			notify_cancelTask(msg->arg);
			continue;
			break;
		default:
			debug(modulo, "IMC->(%d) subcribe: message invalid (%d)\n", pthread_self(), msg->type);
		}
		if (msg->arg != NULL) {
			free(msg->arg);
		}
		free(msg);
	}
}

void startIMC() {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + 1024);

	pthread_create(&thread_run, NULL, (void* (*)(void*)) run, NULL);
	
    pthread_attr_destroy(&attr);
	debug(modulo, "IMC->(%d) startIMC: se inicio el thread (%d)\n", pthread_self(), thread_run);
}

void stopIMC() {
/*
	debug(modulo, "Stop thread IMC\n");
*/
	pthread_cond_broadcast(&enqueueMsg);
	pthread_cancel(thread_run);
}
