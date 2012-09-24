/*
 * dc-messages.h
 *
 *  Created on: 04/11/2010
 *      Author: pmarchesi
 */

#ifndef DC_MESSAGES_H_
#define DC_MESSAGES_H_

#include <time.h>
#include <messages/daemon-daemon.h>
#include "task_states.h"

#define MD5 1
#define TASK_DEFAULT_STATUS NULL

typedef struct resources{
	unsigned long disk;
	unsigned long memory;
	unsigned long cpu;
} Resources;

typedef struct status_task{
	int status;
	int length;
	int method;
	char hash[33];
	char start[51];
	char actual[51];
	char end[51];
}StatusTask;

typedef struct task{
	char id[20];
	char description[40];
	StatusTask status;
	Resources resources;
	struct tasks* dependencies;
} Task;

typedef struct assign_task{
	char nodo[15];
	char id_job[20];
	Task task;
}AssignTask;
typedef struct lala{
	int type;
	unsigned int size;
	void* mem;
	struct lala* next;
}Lala;
typedef struct dependency_net{
	char idTask[20];
	char idDepen[20];
}DependencyNet;

typedef struct tasks{
	Task* task;
	struct tasks* next;
} Tasks;

typedef Tasks Dependencies;

typedef struct target{
	char hash[33];
	int method;
	int length;
} Target;

typedef struct tasks_available{
	Task* task;
	char id_job[20];
	struct tasks_available* next;
} TasksAvailable;

typedef struct task_result{
	char id_job[20];
	char id_task[20];
	int status;
	char value[51];
} TaskResult;

typedef struct job {
	char id[20];
	Target target;
	Tasks* tasks;
} Job;

typedef struct dc_state {
    //esto es para evitar poner una variable global mas
    long conexion_con_interfaz;
    long cant_jobs;
    long cant_task;
    long cant_blocked_task;
    long cant_completed_task;
    long cant_wait_task;
    long cant_running_task;
    char job_id[32];
} DCState;


/* Se cambio por info_nodo
typedef struct node{
	int id;
	unsigned long memory;
	unsigned long disk;
} Node;*/
typedef struct depens_Net {
	DependencyNet depen;
	struct depens_Net* next;
} DependeciesNet;

typedef struct nodes {
	info_nodo* node;
	unsigned long weighted;
	time_t lastHeartbeat;
	TasksAvailable* task;
	struct nodes* next;
} Nodes;

typedef struct nodeWorking{
	info_nodo nodo;
	char task[20];
	time_t lastHeartbeat;
}NodeWorking;

#endif /* DC_MESSAGES_H_ */
