#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include "taskExecuter.h"
#include "threadlocal.h"
#include <string.h>
#include <limits.h>

#include <util/log_ansi.h>
#define modulo "taskExecuter"

__init_logname(modulo)

RunThread* threads = NULL;

void runTask (ThreadData *arg);

void addThread(RunThread* newThread);
void removeThread(pthread_t thread);
RunThread* getThread(pthread_t thread);
RunThread* popThread();

	short taskExecuter_setup(Environment *env){
		;
	}

	Handle taskExecuter_executeTask(TaskRun* task, CALLBACK_FUNCTION *callbackFunc){
		ThreadData* argument = (ThreadData*)malloc(sizeof(ThreadData));
		pthread_t* thread = (pthread_t*) malloc( sizeof(pthread_t));
		argument->task = *task;
		argument->func = *callbackFunc;
		argument->handle.interrupt = DONT_INTERRUPT_TASK;
		argument->handle.next = NULL;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + 1024);

		if (pthread_create(thread, NULL, (void* (*)(void*))runTask, (void *) argument)){
			debug(modulo,"No se pudo iniciar el Thread\n");
			free(argument);
                        Handle null = {NULL};
			return null;
		}
    pthread_attr_destroy(&attr);
		argument->handle.thread = *thread;
		debug(modulo,"Se inicio el Thread: %d   %s\n", thread, task->id_task );
		addThread(&(argument->handle));
                Handle result;
                result.thread = *thread;
                return result;
	}

	void runTask (ThreadData *arg){
		runThreadLocal(arg);
	}

	short taskExecuter_cancelTask(Handle handle){
		debug(modulo,"Cancelar Task %d \n", handle.thread);
		RunThread *toCancel = getThread(handle.thread);
		if (toCancel == NULL){
			debug(modulo, "El Thread no se entraba corriendo %d\n",handle.thread);
			return 0;
		}
		toCancel->interrupt = INTERRUPT_TASK_DONT_SEND_FAIL;
		removeThread(toCancel->thread);
		return pthread_join(toCancel->thread, NULL);
	}

	void taskExecuter_shutdown(void){
		RunThread *remove;
		while (remove = popThread()){ /* /Es = porque si pop le asigna NULL sale*/
			debug(modulo, "Interrumpir %d\n", remove->thread);
			remove->interrupt = INTERRUPT_TASK_DONT_SEND_FAIL; /* /2 es matar y el thread no avisa a CallFunction*/
			pthread_join(remove->thread,NULL);
		}
	}

	void addThread(RunThread* newThread){
		RunThread* next = threads;
		threads = newThread;
		newThread->next = next;
	}

	void removeThread(pthread_t thread){
		RunThread *next, **prev;
		next = threads;
		prev = &threads;
		while (next != NULL){
			if (next->thread == thread){
				(*prev) = next->next;
				return;
			}else{
				prev = &(next->next);
				next = next->next;
			}
		}
	}

	RunThread* getThread(pthread_t thread){
		RunThread *next;
		next = threads;
		while (next != NULL){
			if (next->thread == thread){
				return next;
			}
			next = next->next;
		}
		return NULL;
	}

	RunThread* popThread(){
		RunThread* result;
		if (threads == NULL)
			return NULL;
		result = threads;
		threads = result->next;
		return result;
	}
