/*
 * networkManager.h
 *
 *  Created on: 16/11/2010
 *      Author: pmarchesi
 */

#ifndef NETWORKMANAGER_H_
#define NETWORKMANAGER_H_

#include <messages/daemon-daemon.h>
#include <messages/dc-messages.h>

#define TYPE_PING               0
#define TYPE_PONG               1

#define TYPE_ELECCION           2
#define TYPE_PARTICIPACION      3
#define TYPE_DC_ELECTO          4

#define TYPE_MIGRACION_JOB      5
#define TYPE_MIGRACION_TASK     6
#define TYPE_MIGRACION_DEP      7
#define TYPE_MIGRACION_NODO     8
#define TYPE_MIGRACION_COMPLETE 9

#define TYPE_MIGRACION_DC       10
#define TYPE_DC_CAYO			11
#define TYPE_TASK_RESULT		12

#define TYPE_IPC_I_AM_DC            48
#define TYPE_IPC_DC_MIGRATE         49
#define TYPE_IPC_DC_GET_MEMORY      50
#define TYPE_IPC_ADD_JOB            5
#define TYPE_IPC_ADD_TASK           6
#define TYPE_IPC_ADD_DEP            7
#define TYPE_IPC_ADD_NODO_TASK      8
#define TYPE_IPC_MIGRATION_COMPLETE 9
#define TYPE_IPC_NODO_WORKING       56
#define TYPE_IPC_TASK_RESULT        57
#define TYPE_IPC_ASSIGN_TASK        58
#define TYPE_IPC_NEW_NODO           59
#define TYPE_IPC_ENDED_NODO         60
#define TYPE_IPC_CANCEL_TASK        61
#define TYPE_IPC_DC_MIGRATE_ERROR   62
#define TYPE_IPC_INFO_DC            73
#define TYPE_IPC_INFO_DC_COMPLETE   74

#define TYPE_DISPLAY            101
#define TYPE_STOP               102
#define TYPE_REFRESH            103


#define STATUS_INICIAR_HANDSHAKE 0
#define STATUS_LEER_HEADER 1
#define STATUS_COMPLETAR_HANDSHAKE 2
#define STATUS_CERRAR_CONEXION 3
#define STATUS_TRABAJAR_CONEXION 10


#define TTL 8
#define TIEMPO_SIN_PONG 3

void init_networkManager(char* configName);
void event_startDC();
/*
 * enviar_* Retorna
 * 		0 				Si NO lo pudo enviar
 * 		Otro numero 	Si pudo
 * TODO Los Insomnio_Headers se generan dentro de los mensajes
 */
int enviar_msg_eleccion(char* destino);
int enviar_msg_participacion(char* conexion);
int enviar_msg_dcElecto(char* destino, nodo_to_dc nuevo_dc);
int enviar_msg_ping(char* nodoName, info_nodo info_ping , insomnio_header header);
int enviar_msg_pong(char* nodoName, info_nodo info_ping , insomnio_header header);
int enviar_msg_newTask();
int enviar_msg_cancelTask();
int enviar_msg_working();/** UDP */
int enviar_msg_taskResult(TaskResult* result);


void enviar_msg_nodes();
info_red* red_de_nodos();
void cambiar_el_DC(nodo_to_dc);
void en_eleccion_de_nuevo_DC();
void caida_de_DC_local();
int enviar_estado_del_job(int conexion,Job*);
int enviar_estado_de_la_task(int conexion,Task*);
int enviar_estado_de_la_dependencia(int conexion,DependencyNet*);
int enviar_estado_nodo_tarea(int conexion,NodeWorking*);
int enviar_migracion_completa(int conexion);
bool viene_por_saliente(char* nombreDelNodoQueLlama,info_nodo nodoDestino);
void nuevo_nodo(info_nodo* nodo);
void finalizo_nodo(info_nodo* nodo);

int conectar_a_nodo(char* nodoDestino);

#endif /* NETWORKMANAGER_H_ */
