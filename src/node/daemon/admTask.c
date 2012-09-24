/*
 * admTask.c
 *
 *  Created on: 16/11/2010
 *      Author: pmarchesi
 */
#include <messages/deamon-taskexecuter.h>
#include <node/task-executer/taskExecuter.h>
#include "networkManager.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <util/config.h>
#include <limits.h>
#include <util/log_ansi.h>
#define modulo "admTask"

Handle* taskHandler = NULL;
TaskRun* taskExec = NULL;
pthread_t working;
int checkWorking = 0;

int time_working = 3; /*FIXME: El 3 es magico.. cambiarlo por la data del archivo de conf*/
int fail_send_try_in = 6;

void end_task(TaskResponse* response) {
	TaskResult result;
	info(modulo, "Se finalizo la busqueda de la clave %s estado %s\n", response->value, TASK_NAME(response->status));
	checkWorking = 0;
	strcpy(result.id_task, taskExec->id_task);
	strcpy(result.id_job, taskExec->id_job);
	result.status = response->status;
	strcpy(result.value, response->value);
	free(taskExec);
	taskExec = NULL;
	taskHandler = NULL;
	int send_state = 0;
	while (send_state == 0) {
		debug(modulo, "Envio resultado\n");
		send_state = enviar_msg_taskResult(&result);
		sleep(fail_send_try_in);
	}
}

void run_working() {
	while (1) {
		sleep(time_working);
		if (checkWorking) {
			debug(modulo, "Envio working\n");
			enviar_msg_working();
		}
	}
}

void resolver_msg_newTask(AssignTask* task) {
    info(modulo," Iniciando tarea %s desde %s \n",task->task.id,task->task.status.actual);
	if (taskHandler != NULL) {
		error(modulo, "Me llego otra tarea y estaba trabajando con una. La mato\n");
		checkWorking = 0;
		taskExecuter_cancelTask(*taskHandler);
		free(taskExec);
		taskExec == NULL;
	}
	TaskRun* taskR = (TaskRun*) malloc(sizeof(TaskRun));
	strcpy(taskR->id_task, task->task.id);
	strcpy(taskR->id_job, task->id_job);
	strcpy(taskR->start, task->task.status.actual);
	strcpy(taskR->end, task->task.status.end);
	strcpy(taskR->hash, task->task.status.hash);
	taskR->cpu = task->task.resources.cpu;
	taskR->memory = task->task.resources.memory;
	taskR->disk = task->task.resources.disk;
	taskR->length = task->task.status.length;
	taskR->method = task->task.status.method;
	CALLBACK_FUNCTION func = &end_task;
	taskHandler = &taskExecuter_executeTask(taskR, &func);
	debug(modulo, "taskhanler %d\n", taskHandler);
	taskExec = taskR;
	checkWorking = 1;
}

void resolver_msg_cancelTask() {
    info(modulo," Cancelando tarea \n");
	checkWorking = 0;
	debug(modulo, "Le digo a taskExecuter que cancele %d\n", taskHandler);
	if (taskHandler == 0) {
		debug(modulo, "La tarea ya se habia finalizado\n");
	} else {
		taskExecuter_cancelTask(*taskHandler);
	}
}
void initADMTask(char* configName) {
	char* asd = read_config(configName, "time_working");
	debug(modulo, "time_working %s\n", (asd ? asd : "null"));
	if (read_config(configName, "time_working") == NULL) {
		error(modulo, "time_working no se encuentra en el archivo de conf, se va a tomar 3 segundos\n");
	} else {
		time_working = atoi(read_config(configName, "time_working"));
	}
	
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + 1024);
	
	pthread_create(&working, NULL, (void* (*)(void*)) run_working, NULL);
	
    pthread_attr_destroy(&attr);
}
