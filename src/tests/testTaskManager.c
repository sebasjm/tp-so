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
#include "messages/dc-messages.h"

Job* job = NULL;
int stepJob = 0;
Job* job2 = NULL;
int stepJob2 = 0;
Job* job3 = NULL;
Job* job4 = NULL;

#include <util/log_ansi.h>
#define modulo "testTaskManager"
__init_logname(modulo)

void nm_getInfo(DCState* state){

}
void wm_getInfo(DCState* state){
    
}
void testTM_newNode(void* nodo) {
	debug(modulo, "TestNM->(%d) testTM_newNode\n", pthread_self());
}
void testTM_taskEnded(void* task) {
	debug(modulo, "TestNM->(%d) testTM_taskEnded\n", pthread_self());
}
void testTM_newJob(void* job) {
	debug(modulo, "TestNM->(%d) testTM_newJob\n", pthread_self());
}
void testTM_jobsEnded(void* job) {
	debug(modulo, "TestNM->(%d) testTM_jobsEnded\n", pthread_self());
}
void testTM_availableTasks(void* tasks) {
	debug(modulo, "TestNM->(%d) testTM_availableTasks %d\n", pthread_self(), tasks);
	TasksAvailable* tasksAvailable = (TasksAvailable*) tasks;
	if (tasks == NULL) {
		debug(modulo, "TestNM->(%d) \t\t Tasks es NULL \n", pthread_self());
	}
	if (tasksAvailable != NULL) {
		if (strcmp(tasksAvailable->id_job, job->id) == 0) {
			debug(modulo, "TestNM->(%d) \t\t Tarea(%s) de Job(%s) \n", pthread_self(), tasksAvailable->task->id, tasksAvailable->id_job);
			StatusTask* tar = &(tasksAvailable->task->status);
			debug(modulo, "TestNM->(%d) \t\t Hash(%s) Length(%d) Method(%d) \n", pthread_self(), tar->hash, tar->length, tar->method);

			TaskResult* task = (TaskResult*)malloc(sizeof(TaskResult));
			strcpy(task->id_job, tasksAvailable->id_job);
			strcpy(task->id_task, tasksAvailable->task->id);
			strcpy(task->value, "Pepe");
				debug(modulo, "LALA\n");
			switch (tasksAvailable->task->status.status) {
			case 0:
				if (stepJob != 0) {
					debug(modulo, "TestNM->(%d) TEST1 Fail Step(%d)", pthread_self(), stepJob);
					break;
				}
				stepJob++;
				task->status = TASK_FAIL;
				break;
			case TASK_FAIL:
				if (stepJob != 1)
					debug(modulo, "TestNM->(%d) TEST1 Fail Step(%d)", pthread_self(), stepJob);
				stepJob++;
				task->status = TASK_COMPLETE;
				break;
			default:
				debug(modulo, "TestNM->(%d) TEST1 Fail, Status no esperado Step(%d)", pthread_self(), stepJob);
				break;
			}
			imc_notify(TASK_ENDED, task);

		} else if (strcmp(tasksAvailable->id_job, job2->id) == 0) {
			debug(modulo, "TestNM->(%d) \t\t Tarea(%s) de Job(%s) \n", pthread_self(), tasksAvailable->task->id, tasksAvailable->id_job);
			StatusTask* tar =&(tasksAvailable->task->status);
			debug(modulo, "TestNM->(%d) \t\t Hash(%s) Length(%d) Method(%d) \n", pthread_self(), tar->hash, tar->length, tar->method);

			TaskResult* task = (TaskResult*)malloc(sizeof(TaskResult));
			strcpy(task->id_job , tasksAvailable->id_job);
			strcpy(task->id_task , tasksAvailable->task->id) ;
			strcpy(task->value , "Pepe");
			switch (tasksAvailable->task->status.status) {
			case 0:
				if (stepJob2 != 0) {
					debug(modulo, "TestNM->(%d) TEST2 Fail Step(%d)", pthread_self(), stepJob2);
					break;
				}
				stepJob2++;
				task->status = TASK_FAIL;
				break;
			case TASK_FAIL:
				if (stepJob2 != 1)
					debug(modulo, "TestNM->(%d) TEST2 Fail Step(%d)", pthread_self(), stepJob2);
				stepJob2++;
				task->status = TASK_SUCCESS;
				break;
			default:
				debug(modulo, "TestNM->(%d) TEST2 Fail, Status no esperado Step(%d)", pthread_self(), stepJob2);
				break;
			}
			imc_notify(TASK_ENDED, task);

		} else if (strcmp(tasksAvailable->id_job, job3->id) == 0) {
			debug(modulo, "TestNM->(%d) \t\t Tarea(%s) de Job(%s) \n", pthread_self(), tasksAvailable->task->id, tasksAvailable->id_job);
			StatusTask* tar = &(tasksAvailable->task->status);
			debug(modulo, "TestNM->(%d) \t\t Hash(%s) Length(%d) Method(%d) \n", pthread_self(), tar->hash, tar->length, tar->method);

			TaskResult* task = (TaskResult*)malloc(sizeof(TaskResult));
			strcpy(task->id_job , tasksAvailable->id_job);
			strcpy(task->id_task , tasksAvailable->task->id);
			strcpy(task->value , "Pepe");
			task->status = TASK_COMPLETE;

			imc_notify(TASK_ENDED, task);

		} else if (strcmp(tasksAvailable->id_job, job4->id) == 0) {
			while (tasksAvailable != NULL) {
				debug(modulo, "TestNM->(%d) \t\t Tarea(%s) de Job(%s) \n", pthread_self(), tasksAvailable->task->id, tasksAvailable->id_job);
				StatusTask* tar = &(tasksAvailable->task->status);
				debug(modulo, "TestNM->(%d) \t\t Hash(%s) Length(%d) Method(%d) \n", pthread_self(), tar->hash, tar->length, tar->method);

                                TaskResult* task = (TaskResult*)malloc(sizeof(TaskResult));
                strcpy(task->id_job, tasksAvailable->id_job);
                strcpy(task->id_task, tasksAvailable->task->id);
                strcpy(task->value, "Pepe");
				task->status = TASK_COMPLETE;
				imc_notify(TASK_ENDED, task);
				tasksAvailable = tasksAvailable->next;
			}
		}
	}
}

Task* makeTask(char* id, char* description, unsigned long cpu, unsigned long disk, unsigned long memory, int status, Dependencies* dependencies) {
	Task* task = (Task*)malloc(sizeof(Task));
	debug(modulo, "Depen\n");
	task->dependencies = dependencies;
	debug(modulo, "desc - %s\n", description);
	strcpy(task->description, description);
	debug(modulo, "id\n");
	strcpy(task->id, id);
	debug(modulo, "Atrrib\n");
	task->resources.cpu = cpu;
	task->resources.disk = disk;
	task->resources.memory = memory;

/*
	task->status = status;
*/
	return task;
}

void makeJob1() {
	job = (Job*)malloc(sizeof(Job));
	char* idJ = "Job1";
	char* hashJ = "HASH_JOB1";
	debug(modulo, "Nombres\n");
	strcpy(job->id, idJ);
	strcpy(job->target.hash, hashJ);
	debug(modulo, "Attrib\n");
	job->target.length = 5;
	job->target.method = MD5;
	debug(modulo, "Task\n");
	Task* task = makeTask("JOB1-TASK1", "Desc Job1-T1", 100, 150, 200, 0, NULL);
	debug(modulo, "2\n");
	Tasks* tasks = (Tasks*)malloc(sizeof(Tasks));
	tasks->task = task;
	tasks->next = NULL;
	job->tasks = tasks;
	debug(modulo, "1\n");
}

void makeJob2() {
	job2 = (Job*)malloc(sizeof(Job));
	char* idJ = "Job2";
	char* hashJ = "HASH_JOB2";
	strcpy(job2->id, idJ);
	strcpy(job2->target.hash, hashJ);
	job2->target.length = 7;
	job2->target.method = MD5;
	Task* task = makeTask("JOB2-TASK1", "Desc Job2-T1", 100, 150, 200, 0, NULL);
	Tasks* tasks = (Tasks*)malloc(sizeof(Tasks));
	tasks->task = task;
	tasks->next = NULL;
	job2->tasks = tasks;
}
Dependencies* makeDependecies(Dependencies * depen, Task* task) {
	if (depen == NULL) {
		depen = (Dependencies*)malloc(sizeof(Dependencies));
		depen->task = task;
		depen->next = NULL;
		return depen;
	} else {
		Dependencies* depenNext = makeDependecies(NULL, task);
		depenNext->next = depen;
		return depenNext;
	}
}

Tasks* makeTasks(Tasks* tasks, Task* task) {
	if (tasks == NULL) {
		tasks = (Tasks*)malloc(sizeof(Tasks));
		tasks->task = task;
		tasks->next = NULL;
		return tasks;
	} else {
		Tasks* tasksNext = makeTasks(NULL, task);
		tasksNext->next = tasks;
		return tasksNext;
	}
}

void makeJob3() {
	job3 = (Job*)malloc(sizeof(Job));
	char* idJ = "Job3";
	char* hashJ = "HASH_JOB3";
	strcpy(job3->id , idJ);
	strcpy(job3->target.hash , hashJ);
	job3->target.length = 7;
	job3->target.method = MD5;
	Task* task111 = makeTask("JOB3-TASK1-1-1", "Desc Job3-T111", 100, 150, 200, 0, NULL);
	Dependencies* dep111 = makeDependecies(NULL, task111);

	Task* task11 = makeTask("JOB3-TASK1-1", "Desc Job3-T11", 100, 150, 200, 0, dep111);
	Task* task12 = makeTask("JOB3-TASK1-2", "Desc Job3-T12", 100, 150, 200, 0, NULL);
	Dependencies* dep11 = makeDependecies(NULL, task11);
	Dependencies* dep12 = makeDependecies(dep11, task12);

	Task* task1 = makeTask("JOB3-TASK1", "Desc Job3-T1", 100, 150, 200, 0, dep12);
	Tasks* tasks = makeTasks(NULL, task1);
	tasks = makeTasks(tasks, task11);
	tasks = makeTasks(tasks, task12);
	tasks = makeTasks(tasks, task111);
	job3->tasks = tasks;
}
void makeJob4() {
	job4 = (Job*)malloc(sizeof(Job));
	char* idJ = "Job4";
	char* hashJ = "HASH_JOB4";
	strcpy(job4->id , idJ);
	strcpy(job4->target.hash , hashJ);
	job4->target.length = 5;
	job4->target.method = MD5;
	Task* task111 = makeTask("JOB4-TASK1-1-1", "Desc Job4-T111", 100, 150, 200, 0, NULL);
	Dependencies* dep111 = makeDependecies(NULL, task111);

	Task* task11 = makeTask("JOB4-TASK1-1", "Desc Job4-T11", 100, 150, 200, 0, dep111);
	Task* task12 = makeTask("JOB4-TASK1-2", "Desc Job4-T12", 100, 150, 200, 0, NULL);
	Dependencies* dep11 = makeDependecies(NULL, task11);
	Dependencies* dep12 = makeDependecies(dep11, task12);

	Task* task1 = makeTask("JOB4-TASK1", "Desc Job4-T1", 100, 150, 200, 0, dep12);
	Tasks* tasks = makeTasks(NULL, task1);
	tasks = makeTasks(tasks, task11);
	tasks = makeTasks(tasks, task12);
	tasks = makeTasks(tasks, task111);
	job4->tasks = tasks;
}

int main() {
	initIMC();
	initTM();
	startIMC();
	imc_suscribe(NEW_JOB, (Function*)&testTM_newJob);
	imc_suscribe(TASK_ENDED, (Function*)&testTM_taskEnded);
	imc_suscribe(NEW_NODE, (Function*)&testTM_newNode);
	imc_suscribe(JOBS_ENDED, (Function*)&testTM_jobsEnded);
	imc_suscribe(AVAILABLE_TASKS, (Function*)&testTM_availableTasks);
	debug(modulo, "Creo Job1 \n");
	makeJob1();
	debug(modulo, "Creo Job2 \n");
	makeJob2();
	debug(modulo, "Creo Job3 \n");
	makeJob3();
	debug(modulo, "Creo Job4 \n");
	makeJob4();
	debug(modulo, "Test1 \n");
	imc_notify(NEW_JOB, job);
	sleep(1);
	debug(modulo, "Test2 \n");
	imc_notify(NEW_JOB, job2);
	sleep(1);
	debug(modulo, "Test3 \n");
	imc_notify(NEW_JOB, job3);
	sleep(1);
	debug(modulo, "Test4 \n");
	imc_notify(NEW_JOB, job4);
	sleep(1);
	debug(modulo, "Test5 \n");
	imc_notify(NEW_NODE, NULL);
	sleep(1);
}

