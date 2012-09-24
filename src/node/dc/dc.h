/*
 * dc.h
 *
 *  Created on: 13/10/2010
 *      Author: pmarchesi
 */

#ifndef DC_H_
#define DC_H_
#include <messages/dc-messages.h>
#include <messages/daemon-daemon.h>


void initDC(char** msgid);
void startDC(int migration);

void addJob(Job* job);
void addTask(Task* task);
void addDepen(DependencyNet* depen);
void addNodeTask(NodeWorking* nodeTask);
void migracionComplete();
void newDC();

void sigusrToMigration(int sig);

char* get_dc_ip();
char* get_dc_port();

#endif /* DC_H_ */
