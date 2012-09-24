/*
 * netDiscovery.h
 *
 *  Created on: 16/11/2010
 *      Author: pmarchesi
 */

#ifndef NETDISCOVERY_H_
#define NETDISCOVERY_H_

#include <messages/daemon-daemon.h>

void resolver_msg_pong(char* nodoName,char* nombreNodo, info_nodo info, insomnio_header header);
void resolver_msg_ping(char* configName,char* nodoName,char* nombreNodo, info_nodo info, insomnio_header header);
void broadcast(char* nodoName, info_nodo mi_info, insomnio_header pong);

int enviar_ping_nuevo(char* nodoName, char* configName, char* ip, char* puerto);
char* get_nombre_saliente();

int reconectar_a_otro_saliente(nodo_red* nodo_caido);

#endif /* NETDISCOVERY_H_ */
