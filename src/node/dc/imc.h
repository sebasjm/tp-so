/*
 * imc.h
 *
 *  Created on: 30/10/2010
 *      Author: pmarchesi
 */

#ifndef IMC_H_
#define IMC_H_

#define NEW_JOB 		 		1
#define TASK_ENDED 				2
#define NEW_NODE 				3
#define JOBS_ENDED 				4
#define AVAILABLE_TASKS		 	5
#define FAIL_TASK_NODE 			6
#define MIGRATE					7
#define ADD_JOB					8
#define ADD_TASK				9
#define ADD_DEP				   	10
#define ADD_NODE_TASK 			11
#define MIGRATION_COMPLETED		12
#define NODE_WORKING			13
#define NODE_ENDED				14
#define ASSIGN_TASK				15
#define CANCEL_TASK				16
#define GET_INFO_DC				20
#define GET_INFO_COMPLETE			21



typedef struct message{
	int type;
	void* arg;
	struct message* next;
} Message;

typedef void (* Function)(void *);

typedef struct element{
	Function element;
	struct element* next;
} Element;

void initIMC();
void startIMC();
void stopIMC();

#include <messages/dc-messages.h>

void tm_getInfo(DCState* state);
void nm_getInfo(DCState* state);
void wm_getInfo(DCState* state);

void imc_suscribe(int type, Function* function);

void imc_notify(int type, void* arg);



#endif /* IMC_H_ */
