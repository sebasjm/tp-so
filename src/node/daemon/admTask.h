/*
 * admTask.h
 *
 *  Created on: 16/11/2010
 *      Author: pmarchesi
 */

#ifndef ADMTASK_H_
#define ADMTASK_H_
#include <messages/dc-messages.h>

void initADMTask(char* configName);
void resolver_msg_newTask(AssignTask* task);
void resolver_msg_cancelTask();


#endif /* ADMTASK_H_ */
