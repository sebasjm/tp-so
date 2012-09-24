#ifndef __daemon_taskexecuter_h
#define __daemon_taskexecuter_h

#include <pthread.h>
#include "task_states.h"


typedef struct a_handle{
	pthread_t thread;
} Handle;

typedef struct run_handle{
	pthread_t thread;
	int interrupt;
	struct run_handle* next;
} RunThread;


typedef struct a_environment{
	void* noEstaDefinido;
} Environment;

typedef struct a_task {
	char id_task[20];
	char id_job[20];
	char hash[33];
	char start[51];
	char end[51];
	int method;
	int length;
	long disk;
	long memory;
	long cpu;
} TaskRun;

typedef struct a_task_result{
	Handle handle;
	short   status;
	char  value[51];
} TaskResponse;

typedef void (* CALLBACK_FUNCTION)(TaskResponse *);

typedef struct thread_data{
   TaskRun  task;
   CALLBACK_FUNCTION  func;
   RunThread handle;
} ThreadData;


#endif
