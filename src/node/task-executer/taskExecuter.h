#ifndef __task_executer_h
#define __task_executer_h

#include <messages/deamon-taskexecuter.h>

#define DONT_INTERRUPT_TASK 			0
#define INTERRUPT_TASK_SEND_FAIL 		1
#define INTERRUPT_TASK_DONT_SEND_FAIL 	2

short taskExecuter_setup(Environment *);
	/*A trav�s de esta llamada el componente es inicializado con datos del entorno que se consideren
	requeridos para poder operar. Aqu� se ejecuta el c�digo de inicializaci�n de la biblioteca. Recibe como
	par�metro la direcci�n de memoria de una estructura que agrupa dicha configuraci�n.*/

Handle taskExecuter_executeTask(TaskRun *, CALLBACK_FUNCTION *);
	/*Se le indica al Task Executer que comience la ejecuci�n de una Task que es enviada por par�metro. Tal
	estructura contiene la informaci�n necesaria para llevar a cabo dicha ejecuci�n. El segundo par�metro es
	la direcci�n de la rutina que ser� llamada cuando la tarea finalice. Retorna el descriptor de la tarea en
	ejecuci�n.
	Esta llamada se ejecutar� de forma as�ncrona, esto significa que no bloquear� al cliente que la utilice,
	retornando inmediatamente luego de llamarla. El tipo de dato CALLBACK_FUNCTION corresponde con el
	siguiente alias: typedef void (* CALLBACK_FUNCTION)(void *); donde el puntero que se recibe por
	par�metro corresponde con el resultado de la operaci�n.*/

short taskExecuter_cancelTask(Handle);
	/*Esta llamada cancela una tarea en ejecuci�n. Se recibe como par�metro el descriptor de dicha tarea.
	Retorna el resultado de dicha operaci�n.*/

void taskExecuter_shutdown(void);
	/*En una finalizaci�n normal, el cliente debe llamar a esta rutina la cual se encarga de liberar los recursos
	utilizados por el Task Executer.*/
void removeThread(pthread_t thread);
#endif
