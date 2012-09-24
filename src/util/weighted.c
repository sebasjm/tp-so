/*
 * weigthed.c
 *
 *  Created on: 18/11/2010
 *      Author: pmarchesi
 */
#ifndef __weighted_c
#define __weighted_c

#include <util/weighted.h>

unsigned long getWeighted(unsigned long memory, unsigned long disk) {
	return (memory * 7 + disk);/*TODO Algoritmo de Ponderacion*/
}
unsigned long getWeightedNodo(info_nodo node) {
	return getWeighted(node.memoria, node.disco);
}
unsigned long getMultiplierUnit(char* unit) {
	if ((strcmp(unit, "KB") == 0) || (strcmp(unit, "Kb") == 0) || (strcmp(unit, "kB") == 0) || (strcmp(unit, "kb") == 0)) {
		return 1024;
	}
	if ((strcmp(unit, "MB") == 0) || (strcmp(unit, "Mb") == 0) || (strcmp(unit, "mB") == 0) || (strcmp(unit, "mb") == 0)) {
		return 1024 * 1024;
	}
	if ((strcmp(unit, "GB") == 0) || (strcmp(unit, "Gb") == 0) || (strcmp(unit, "gB") == 0) || (strcmp(unit, "gb") == 0)) {
		return 1024 * 1024 * 1024;
	}
	return 1;
}

#endif
