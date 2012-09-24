#ifndef _daemon_commons_h__
#define _daemon_commons_h__

#include <messages/daemon-daemon.h>

nodo_red* buscar_nodo_en_red(char* nodoDestino);
nodo_red* buscar_nodo_en_red_por_sock(int connection);
nodo_red* buscar_dc();
//char* get_nombre_saliente();
void agregar_al_final(nodo_red* nodo, nodo_red*& lista);
//char* get_nombre_saliente();
nodo_red* buscar_nodo_por_nombre(char* nodoDestino, nodo_red* lista);
bool ya_fue_buscado( nodo_red* nodo, nodo_red* lista);

void activar_refresco_de_red_por_intervalo(char* configName, char* ip, char* puerto);

#endif 
