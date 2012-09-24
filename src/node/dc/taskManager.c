/*
 * taskManager.c
 *
 *  Created on: 13/10/2010
 *      Author: pmarchesi
 */
#include "imc.h"
#include <stdio.h>
#include <string.h>
#include "util/characters.c"
#include <pthread.h>
#include <stdlib.h>
#include "taskManager.h"

#include <util/log_ansi.h>
#define modulo "taskManager"

/*TODO: free message*/
typedef struct jobs {
	Job* job;
	int ended;
	struct jobs* next;
} Jobs;

Jobs* jobsQueue = NULL;

int tm_enableToExecute(Task* task) {
	debug(modulo, "TM->(%d) enableToExecute %s \n", pthread_self(), task->id);
	if ((task->status.status != TASK_COMPLETE) && (task->status.status != TASK_SUCCESS)) {
		debug(modulo, "TM->(%d) enableToExecute: Chequear Dependencia \n", pthread_self());
		Dependencies* depen = task->dependencies;
		while (depen != NULL) {
			debug(modulo, "TM->(%d) enableToExecute: Dependencia con %s \n", pthread_self(), depen->task->id);
			if (depen->task->status.status != TASK_COMPLETE) {
				debug(modulo, "TM->(%d) enableToExecute: Dependencia Incumplida \n", pthread_self());
				return 1;
			} else {
				debug(modulo, "TM->(%d) enableToExecute: Dependencia Cumplida \n", pthread_self());
			}
			depen = depen->next;
		}
		debug(modulo, "TM->(%d) La Tarea %s tiene las dependencias cumplidas \n", pthread_self(), task->id);
		return 0;
	}
	debug(modulo, "TM->(%d) \t La Tarea no se realiza (status %s) \n", pthread_self(), TASK_NAME(task->status.status));
	return 1;
}

void tm_checkJob(Jobs* jobQ) {
	debug(modulo, "CHECK-JOB inicio %s\n", jobQ->job->id);
	Job* job = jobQ->job;
	Tasks* pointTask = job->tasks;
	if (jobQ->ended == 0){
	while (pointTask != NULL) {
		if (tm_enableToExecute(pointTask->task) == 0) {
			debug(modulo, "CHECK-JOB Job %s NO Termino\n", jobQ->job->id);
			return;
		}
		pointTask = pointTask->next;
		}
	}
	jobQ->ended = 1;
	debug(modulo, "CHECK-JOB Job %s Termino\n", jobQ->job->id);
	imc_notify(JOBS_ENDED, job);
}
int tm_sizeOfTask(Tasks* tasks) {
	int size = 0;
	while (tasks != NULL) {
		size++;
		tasks = tasks->next;
	}
	return size;
}
Task* tm_getTaskIndex(Tasks* tasks, int initTask) {
	int index = 0;
	while (index < initTask) {
		index++;
		tasks = tasks->next;
	}
	return tasks->task;
}

void tm_addCharsInEnd(char* str, int length, char value) {
	int pos = 1;
	while (length > pos) {

		str[pos] = value;
		pos++;
	}
	str[pos] = '\0';
}
void tm_calculeStatusOfTasks(Job* job) {
	int sizeTasks = tm_sizeOfTask(job->tasks);
	int numberPass = (int) (numberOfCharactares / sizeTasks);
	int initTask = 0;
	while (initTask < sizeTasks) {
		Task* task = tm_getTaskIndex(job->tasks, initTask);
		char* start = (char*) malloc(job->target.length + 1);
		char* end = (char*) malloc(job->target.length + 1);
		char* actual = (char*) malloc(job->target.length + 1);

		start[0] = characters[initTask * numberPass];
		tm_addCharsInEnd(start, job->target.length, characters[0]);
		strcpy(actual, start);
		if ((initTask + 1) != sizeTasks) {
			end[0] = characters[((initTask + 1) * numberPass) - 1];
		} else {
			end[0] = characters[numberOfCharactares - 1];
		}
		tm_addCharsInEnd(end, job->target.length, characters[numberOfCharactares - 1]);

		debug(modulo, "Task %s  \tStart %s \tEnd %s\n", task->id, start, end);

		strcpy(task->status.start, start);
		strcpy(task->status.end, end);
		strcpy(task->status.actual, actual);
		task->status.status = 0;
		task->status.length = job->target.length;
		task->status.method = job->target.method;
		strcpy(task->status.hash, job->target.hash);
		initTask++;
	}
}

TasksAvailable* tm_getTasks() {
	debug(modulo, "TM->(%d) getTasks \n", pthread_self());
	TasksAvailable* tasksResult = NULL;
	Jobs* pointJobs = jobsQueue;
	while (pointJobs != NULL) {
		debug(modulo, "TM->(%d) \t Un Job %s  estado %d\n", pthread_self(), pointJobs->job->id, pointJobs->ended);
		if (pointJobs->ended == 0) {
			debug(modulo, "TM->(%d) \t No terminado \n", pthread_self());
			Tasks* pointTask = pointJobs->job->tasks;
			while (pointTask != NULL) {
				debug(modulo, "TM->(%d) \t Hay una Tarea (%s)\n", pthread_self(), pointTask->task->id);
				if (tm_enableToExecute(pointTask->task) == 0) {
					debug(modulo, "TM->(%d) \t Y se va a agregar a la Cola Available \n", pthread_self());
					TasksAvailable* newTask = (TasksAvailable*) malloc(sizeof(TasksAvailable));
					newTask->task = pointTask->task;
					strcpy(newTask->id_job, pointJobs->job->id);
					newTask->next = tasksResult;
					tasksResult = newTask;
				}
				pointTask = pointTask->next;
			}
		}
		pointJobs = pointJobs->next;
	}
	return tasksResult;
}

void tm_notify_available_tasks() {
	TasksAvailable* tasks = tm_getTasks();
	debug(modulo, "TM->(%d) tm_notify_available_tasks:  TasksAvailable %d\n", pthread_self(), tasks);
	if (tasks == NULL)
		return;
	imc_notify(AVAILABLE_TASKS, tasks);
}
void tm_removeJob(Jobs* job) {
	Jobs* point = jobsQueue;
	if (jobsQueue == NULL)
		return;
	if (strcmp(jobsQueue->job->id, job->job->id) == 0) {
		jobsQueue = jobsQueue->next;
		return;
	}
	while (point->next != NULL) {
		if (strcmp(point->next->job->id, job->job->id) == 0) {
			point->next = point->next->next;
			return;
		}
	}
	error(modulo, "No se encotro el job para quitarlo");
}
Jobs* tm_getJob(char* id_job) {
	Jobs* point = jobsQueue;

	while (point != NULL) {

		if (strcmp(id_job, point->job->id) == 0)
			return point;

		point = point->next;
	}
	return NULL; /* No esta ese id_job */
}

void tm_enqueueJob(Jobs* job) {
	Jobs* point = jobsQueue;
	job->next = NULL;
	if (point == NULL) {
		jobsQueue = job;
	} else {
		while (point->next != NULL) {
			point = point->next;
		}
		point->next = job;
	}
}

void tm_changeNodo(void* nodo) {
	info(modulo, "Node change\n", pthread_self());
	tm_notify_available_tasks();
}

void tm_taskEnded(void* taskEnded) {
	TaskResult* task_result = (TaskResult*) taskEnded;
	info(modulo, "TASK-ENDED job: %s task: %s status: %s value:%s\n", task_result->id_job, task_result->id_task, TASK_NAME( task_result->status) , task_result->value);
	Jobs* job = tm_getJob((char*) task_result->id_job);
	if ((task_result->status) == TASK_SUCCESS) {
		if (job == NULL) {
			debug(modulo, "TM-> tm_taskEnded: ERROR - Job es NULO (id %d)\n", task_result->id_job);
		} else {
			debug(modulo, "Marco en Job %s como finalizado\n", job->job->id);
			job->ended = 1;
		}
	}

	if (job != NULL) {
		Tasks* tasks = job->job->tasks;
		while (strcmp(task_result->id_task, (tasks->task)->id) != 0) {
			tasks = tasks->next;
		}
		(tasks->task)->status.status = task_result->status;
		strcpy((tasks->task)->status.actual, task_result->value);
		tm_checkJob(job);
		if (job->ended == 1) {
			tm_removeJob(job);
		}
	} else {
		debug(modulo, "TM-> tm_taskEnded: El Job no se encuentra, no se puede actualizar la tarea (%s-%s)\n", task_result->id_job, task_result->id_task);
	}
	tm_notify_available_tasks();
	debug(modulo, "BORRARLALA (%s-%s)\n", task_result->id_job, task_result->id_task);
}

void tm_newJob(void* job) {
	Jobs* jobToQueue = (Jobs*) malloc(sizeof(Jobs*));
	tm_calculeStatusOfTasks((Job*) job);
	jobToQueue->job = (Job*) job;
	jobToQueue->ended = 0;
	jobToQueue->next = NULL;
	info(modulo, "NEW-JOB id: %s\n", jobToQueue->job->id);
	tm_enqueueJob(jobToQueue);
	tm_notify_available_tasks();
}
void tm_getInfo(DCState* state) {
    debug(modulo,"GET-INFO Adjuntando mi informacion, le paso la bola al NodoManager\n");
    Tasks* tmp = NULL;
    if (jobsQueue != NULL && jobsQueue->job != NULL) 
        tmp = jobsQueue->job->tasks;
    
    int count = 0;
    int completed = 0;
    int blocked = 0;
    while(tmp) {
        count++;
        if ( tmp->task->status.status == TASK_COMPLETE ) 
            completed++;
            
        if ( tmp->task->status.status != TASK_COMPLETE && tmp->task->status.status != TASK_SUCCESS && tm_enableToExecute( tmp->task ) == 1 )
            blocked++;
        
        tmp = tmp->next;
    }
    state->cant_task = count;
    state->cant_completed_task = completed;
    state->cant_blocked_task = blocked;

   TasksAvailable* tasks = tm_getTasks();
   count = 0;
   while(tasks) {
       count++;
       tasks = tasks->next;
   }
   state->cant_wait_task = count - state->cant_running_task;
}
void initTM() {
	imc_suscribe(NEW_NODE, (Function*) &tm_changeNodo);
	imc_suscribe(NODE_ENDED, (Function*) &tm_changeNodo);
	imc_suscribe(TASK_ENDED, (Function*) &tm_taskEnded);
	imc_suscribe(NEW_JOB, (Function*) &tm_newJob);
}

Job* getWorkingJob() {
	Jobs* pointJobs = jobsQueue;
	while (pointJobs != NULL) {
		if (pointJobs->ended == 0) {
			return pointJobs->job;
		}
/*
		debug(modulo, "Miro el siguiente\n");
*/
		pointJobs = pointJobs->next;
	}
	return NULL;
}

void setJobTM(Job* job){
	Jobs* jobQ = (Jobs*) malloc(sizeof(Jobs));
	jobQ->job = job;
	jobQ->ended = 0;
	jobQ->next =NULL;
	jobsQueue = jobQ;
}
