/*
 * migration.c
 *
 *  Created on: 16/11/2010
 *      Author: pmarchesi
 */
#include "networkManager.h"
#include "migration.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <util/weighted.h>
#include <util/config.h>

int enElecion = 0;
char migracion = '0';
info_nodo* bestNodeDC = NULL;
pthread_mutex_t bestNodeDCMutex;
int tiempoDeEleccion = 2;

#include <util/log_ansi.h>
#define modulo "migration"

void init_migration(char* configName) {
	pthread_mutex_init(&bestNodeDCMutex, NULL);
}
/**
 * Metodo Sincronizado.
 * Cambia el Nodo enviado solo si tiene menos ponderacion que el actual
 */
void cambiar_mejor_nodo(info_nodo info) {
	pthread_mutex_lock(&bestNodeDCMutex);
	if (bestNodeDC == NULL) {
		info_nodo* newNodo = (info_nodo*)malloc(sizeof(info_nodo));
		(*newNodo) = info;
		bestNodeDC = newNodo;
		debug(modulo, "bestNodeDC nulo. Nuevo DC temporal %s\n", bestNodeDC->nombre);
	} else {
		debug(modulo, "bestNodeDC %lu    new %lu\n", getWeightedNodo(*bestNodeDC), getWeightedNodo(info));
		if (getWeightedNodo(info) < getWeightedNodo(*bestNodeDC)) {
			info_nodo* newNodo = (info_nodo*)malloc(sizeof(info_nodo));
			(*newNodo) = info;
			info_nodo* tmp = bestNodeDC;
			bestNodeDC = newNodo;
			debug(modulo, "bestNodeDC tenia menos recursos. Nuevo DC temporal %s\n", bestNodeDC->nombre);
			free(tmp);
		}
	}
	pthread_mutex_unlock(&bestNodeDCMutex);
}

void en_eleccion() {
	pthread_mutex_lock(&bestNodeDCMutex);
	enElecion = 1;
	pthread_mutex_unlock(&bestNodeDCMutex);
	en_eleccion_de_nuevo_DC();
}

void finalizo_eleccion() {
	pthread_mutex_lock(&bestNodeDCMutex);
	enElecion = 0;
	pthread_mutex_unlock(&bestNodeDCMutex);
}

void esperar_por_respuestas() {
	sleep(tiempoDeEleccion);
        info(modulo,"Tiempo de eleccion terminado\n");

    if (enElecion == 1){
		pthread_mutex_lock(&bestNodeDCMutex);
		info_nodo* nodoWin = bestNodeDC;
		bestNodeDC = NULL;
		enElecion = 0;
		pthread_mutex_unlock(&bestNodeDCMutex);
		nodo_to_dc nodoToDC = {*nodoWin, migracion};
		info_red* red = red_de_nodos();
		enviar_msg_dcElecto(red->mi_nodo->info.nombre, nodoToDC);
		nodo_red* nodo = red->nodos_entrantes;
		debug(modulo, "Nodo Electo %s\n", nodoWin->nombre);

		while (nodo != NULL) {
			enviar_msg_dcElecto(nodo->info.nombre, nodoToDC);
			nodo = nodo->next;
		}
		nodo = red->nodos_salientes;
		while (nodo != NULL) {
			enviar_msg_dcElecto(nodo->info.nombre, nodoToDC);
			nodo = nodo->next;
		}
    }
}

void resolver_caida_de_DC(info_nodo* caido, char mig) {
	info(modulo, "Resolviendo caida del DC\n");
	if (enElecion != 0 && bestNodeDC != NULL) {
		info(modulo, "Me encuentro en Eleccion, no hago nada \n");
		return;
	}
	debug(modulo, "No me encuentro en Eleccion\n");
	info_nodo thisNode = red_de_nodos()->mi_nodo->info;
	cambiar_mejor_nodo(thisNode);
	en_eleccion();
	migracion = mig;

	info_red* red = red_de_nodos();

	nodo_red* nodo = red->nodos_entrantes;
	debug(modulo, "Nodos Entrantes, Hay? %s\n", (nodo==NULL?"NO":"SI"));
	while (nodo != NULL) {
		if ((caido == NULL) || (strcmp(nodo->info.nombre, caido->nombre) != 0)) {
			if (getWeightedNodo(nodo->info) < getWeightedNodo(thisNode)) {
				info(modulo, "    Envio Eleccion a %s\n", nodo->info.nombre);
				enviar_msg_eleccion(nodo->info.nombre);
			}
		}
		nodo = nodo->next;
	}
	nodo = red->nodos_salientes;
	debug(modulo, "Nodos Saliente, Hay? %s\n", (nodo==NULL?"NO":"SI"));
	while (nodo != NULL) {
		if ((caido == NULL) || (strcmp(nodo->info.nombre, caido->nombre) != 0)) {
			if (getWeightedNodo(nodo->info) < getWeightedNodo(thisNode)) {
				info(modulo, "    Envio Eleccion a %s\n", nodo->info.nombre);
				enviar_msg_eleccion(nodo->info.nombre);
			}
		}
		nodo = nodo->next;
	}
	pthread_t esperarPorRespuestas;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + 1024);

	pthread_create(&esperarPorRespuestas, NULL, (void* (*)(void*)) esperar_por_respuestas,NULL);
	
    pthread_attr_destroy(&attr);
}

void resolver_msg_eleccion(char* conexion) {
	/*Se podria agregar condicionar en el archivo de configuracion*/
	en_eleccion();
	debug(modulo, "Respondo con participacion\n");
	enviar_msg_participacion(conexion);
}

void resolver_msg_participacion(info_nodo n_info) {
	info(modulo, "Recibiendo participante (%s)\n", n_info.nombre);
	cambiar_mejor_nodo(n_info);
}

void resolver_msg_dcElecto(nodo_to_dc nodo) {
	info(modulo, "Nuevo DC %s %d\n", nodo.info.nombre, nodo.migracion);
	cambiar_el_DC(nodo);
	finalizo_eleccion();
	migracion = 0;
}

