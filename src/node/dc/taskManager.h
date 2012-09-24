/*
 * taskManager.h
 *
 *  Created on: 13/10/2010
 *      Author: pmarchesi
 */

#ifndef TASKMANAGER_H_
#define TASKMANAGER_H_
#include "messages/dc-messages.h"

void initTM();
void startTM();
void stopTM();
Job* getWorkingJob();
void setJobTM(Job*);

#endif /* TASKMANAGER_H_ */
