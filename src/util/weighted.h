/*
 * weigthed.c
 *
 *  Created on: 18/11/2010
 *      Author: pmarchesi
 */
#ifndef __weighted_h
#define __weighted_h

#include <messages/daemon-daemon.h>
#include <string.h>

unsigned long getWeighted(unsigned long memory, unsigned long disk);
unsigned long getWeightedNodo(info_nodo node);
unsigned long getMultiplierUnit(char* unit);

#endif
