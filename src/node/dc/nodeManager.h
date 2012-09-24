/*
 * nodoManager.h
 *
 *  Created on: 13/10/2010
 *      Author: pmarchesi
 */

#ifndef NODOMANAGER_H_
#define NODOMANAGER_H_
#include <messages/dc-messages.h>


void initNM();
void startNM();
void stopNM();

Nodes* getNodesAvailables();
Nodes* getNodesWorking();
void addNodoTaskNM(Nodes*);
#endif /* NODOMANAGER_H_ */
