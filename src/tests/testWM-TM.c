/*
 * testTaskManager.c
 *
 *  Created on: 05/11/2010
 *      Author: pmarchesi
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <node/dc/imc.h>
#include <node/dc/taskManager.h>
#include <node/dc/webManager.h>
#include "messages/dc-messages.h"

#include <util/log_ansi.h>
#define modulo "testWM-TM"
__init_logname(modulo)


void nm_getInfo(DCState* state) {

}
void testWM_TM_newNode(void* nodo) {
	debug(modulo, "TestNM->(%d) testTM_newNode\n", pthread_self());
}
void testWM_TM_taskEnded(void* task) {
	debug(modulo, "TestNM->(%d) testTM_taskEnded\n", pthread_self());
}
void testWM_TM_newJob(void* job) {
	debug(modulo, "TestNM->(%d) testTM_newJob\n", pthread_self());
}
void testWM_TM_jobsEnded(void* job) {
	debug(modulo, "TestNM->(%d) testTM_jobsEnded\n", pthread_self());
}

void testWM_TM_availableTasks(void* tasks) {
	debug(modulo, "TestNM->(%d) testTM_availableTasks %d\n", pthread_self(), tasks);
	TasksAvailable* tasksAvailable = (TasksAvailable*) tasks;
	if (tasks == NULL) {
		debug(modulo, "TestNM->(%d) \t\t Tasks es NULL \n", pthread_self());
	}
	if (tasksAvailable != NULL) {
		debug(modulo, "TestNM->(%d) \t\t Tarea(%s) de Job(%s) \n", pthread_self(), tasksAvailable->task->id, tasksAvailable->id_job);
		StatusTask* tar = &(tasksAvailable->task->status);
		debug(modulo, "TestNM->(%d) \t\t Hash(%s) Length(%d) Method(%d) \n", pthread_self(), tar->hash, tar->length, tar->method);

		TaskResult* task = (TaskResult*)malloc(sizeof(TaskResult));
		strcpy(task->id_job, tasksAvailable->id_job);
		strcpy(task->id_task, tasksAvailable->task->id);
		strcpy(task->value, "Test");

                int len = strlen(task->id_job);
                if ( (task->id_job[len-1]%3) == 0 ) {
                    task->status = TASK_SUCCESS;
                    strcpy(task->value,"testing will save your life!");
                } else {
                    if ( random()%2 ) {
                        task->status = TASK_COMPLETE;
                        strcpy(task->value,"xxx");
                    } else {
                        task->status = TASK_FAIL;
                        strcpy(task->value,"xxx");
                    }
                }

		imc_notify(TASK_ENDED, task);

	}

}

int main() {
    pthread_mutex_init(&mutexLog,NULL);
	initIMC();
	initTM();
	initWM();
	imc_suscribe(NEW_JOB, (Function*)&testWM_TM_newJob);
	imc_suscribe(TASK_ENDED, (Function*)&testWM_TM_taskEnded);
	imc_suscribe(NEW_NODE, (Function*)&testWM_TM_newNode);
	imc_suscribe(JOBS_ENDED, (Function*)&testWM_TM_jobsEnded);
	imc_suscribe(AVAILABLE_TASKS, (Function*)&testWM_TM_availableTasks);
	startIMC();
	startWM();

	sleep(60);
	printf("ok\n");
}

