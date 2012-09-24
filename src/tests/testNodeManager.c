/*
 * testNodoManager.c
 *
 *  Created on: 13/10/2010
 *      Author: pmarchesi
 */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <node/dc/imc.h>
#include "messages/dc-messages.h"
#include <node/dc/nodeManager.c>
#include <util/weighted.c>

#include <util/log_ansi.h>
#define modulo "testNodeManager"
__init_logname(modulo)


/*TODO falta que se de cuenta que la tarea que viene se esta ejecutando o no!!*/
void testNM_newNode(void* nodo) {
	debug(modulo, "TestNM->(%d) testTM_newNode\n", pthread_self());
}
void testNM_taskEnded(void* task) {
	debug(modulo, "TestNM->(%d) testTM_taskEnded\n", pthread_self());
}
void testNM_newJob(void* job) {
	debug(modulo, "TestNM->(%d) testTM_newJob\n", pthread_self());
}
void testNM_jobsEnded(void* job) {
	debug(modulo, "TestNM->(%d) testTM_jobsEnded\n", pthread_self());
}
void testNM_availableTasks(void* tasks) {
	debug(modulo, "TestNM->(%d) testNM_availableTasks\n", pthread_self());
}

void tm_getInfo(DCState* state) {

}
void wm_getInfo(DCState* state) {
    
}


Task* makeTask(char* id, char* description, unsigned long cpu, unsigned long disk, unsigned long memory, int status, Dependencies* dependencies) {
	Task* task = (Task*)malloc(sizeof(Task));
	task->dependencies = dependencies;
	strcpy(task->description, description);
	strcpy(task->id, id);
	task->resources.cpu = cpu;
	task->resources.disk = disk;
	task->resources.memory = memory;
	task->status.status = status;
	return task;
}

int main() {
	initIMC();
	initNM();
	imc_suscribe(NEW_JOB, (Function*)&testNM_newJob);
	imc_suscribe(TASK_ENDED, (Function*)&testNM_taskEnded);
	imc_suscribe(NEW_NODE, (Function*)&testNM_newNode);
	imc_suscribe(JOBS_ENDED, (Function*)&testNM_jobsEnded);
	imc_suscribe(AVAILABLE_TASKS, (Function*)&testNM_availableTasks);
	startIMC();

	info_nodo* node3 = (info_nodo*)malloc(sizeof(info_nodo));
	strcpy(node3->nombre,"Nodo3");
	node3->disco = 15000;
	node3->memoria = 150000;
	imc_notify(NEW_NODE, node3);

	info_nodo* node = (info_nodo*)malloc(sizeof(info_nodo));
	strcpy(	node->nombre, "Nodo1");
	node->disco = 20000;
	node->memoria= 100000;
	imc_notify(NEW_NODE, node);

	info_nodo* node2 = (info_nodo*)malloc(sizeof(info_nodo));
	strcpy(node2->nombre ,"Nodo2");
	node2->disco = 5000;
	node2->memoria= 50000;
	imc_notify(NEW_NODE, node2);

	info_nodo* node4 = (info_nodo*)malloc(sizeof(info_nodo));
	strcpy(node4->nombre ,"Nodo2");
	node4->disco = 21000;
	node4->memoria = 50000;
	imc_notify(NEW_NODE, node4);

	Task* task = makeTask("Tarea1", "DescTarea1", 100, 1000, 4000, 0, NULL);
	TasksAvailable* tasks = (TasksAvailable*)malloc(sizeof(TasksAvailable));
	char* idjob = "Job01";
	strcpy(tasks->id_job, idjob);
	tasks->next = NULL;
	tasks->task = task;
	tasks->task->status.status = TASK_DEFAULT_STATUS;
	strcpy(tasks->task->status.actual, "");
	strcpy(tasks->task->status.end, "");
	strcpy(tasks->task->status.start, "");

	imc_notify(AVAILABLE_TASKS, tasks);
	sleep(1);
	Task* task2 = makeTask("Tarea2", "DescTarea", 100, 20500, 4000, 0, NULL);
	TasksAvailable* tasks2 = (TasksAvailable*)malloc(sizeof(TasksAvailable));
	char* idjob2 = "Job02";
	strcpy(tasks2->id_job, idjob2);
	tasks2->next = NULL;
	tasks2->task = task2;
	imc_notify(AVAILABLE_TASKS, tasks2);
	sleep(1);
	TaskResult* res = (TaskResult*)malloc(sizeof(TaskResult));
	strcpy(res->id_job, idjob);
	res->status = 1;
	strcpy(res->id_task, task->id);
	strcpy(res->value, node2->nombre);
	imc_notify(TASK_ENDED, res);
	sleep(1);
	TaskResult* res2 = (TaskResult*)malloc(sizeof(TaskResult));
	strcpy(res->id_job, idjob2);
	res->status = 1;
	strcpy(res->id_task, task2->id);
	strcpy(res->value, node2->nombre);
	imc_notify(TASK_ENDED, res2);
	sleep(3);
}
