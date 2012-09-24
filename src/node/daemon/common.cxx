#include "netDiscovery.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>

#include <ipc/socket.h>
#include <ipc/socketBuilder.h>
#include "handshake.h"
#include <util/config.h>
#include "networkManager.h"
#include <limits.h>

#include <node/daemon/common.h>
#include <util/config.h>

using namespace std;

#include <util/log.h>

__init_log(daemon_common)

bool ya_fue_buscado(nodo_red* nodo, nodo_red* lista) {
	return buscar_nodo_por_nombre(nodo->info.nombre, lista) != NULL;
}

nodo_red* buscar_nodo_por_nombre(char* nodoDestino, nodo_red* lista) {
	nodo_red* nodo = NULL;

	if (!nodoDestino)
		return NULL;
	//si no lo encontro, lo busco en las salientes
	for (nodo = lista; nodo != NULL && strcmp(nodo->info.nombre, nodoDestino); nodo = nodo->next) {
	}

	return nodo;
}

char* get_nombre_saliente() {
	nodo_red* ptr;
	for (ptr = red_de_nodos()->nodos_salientes; ptr != NULL; ptr = ptr->next) {
		if (ptr->info.distancia != 1)
			return ptr->info.nombre;
	}
	return "NO!";
}

void agregar_al_final(nodo_red* nodo, nodo_red*& lista) {
	nodo_red* tmp = lista;
	while (tmp != NULL && tmp->next != NULL)
		tmp = tmp->next;
	if (lista == NULL) {
		lista = nodo;
	} else {
		tmp->next = nodo;
	}
}

nodo_red* buscar_nodo_en_red(char* nodoDestino) {
	nodo_red* nodo = NULL;

	if (!strcmp(nodoDestino, red_de_nodos()->mi_nodo->info.nombre))
		return red_de_nodos()->mi_nodo;

	//busco en conexiones entrantes
	for (nodo = red_de_nodos()->nodos_entrantes; nodo != NULL && strcmp(nodo->info.nombre, nodoDestino); nodo = nodo->next) {

	}

	if (nodo != NULL)
		return nodo;
	//si no lo encontro, lo busco en las salientes
	for (nodo = red_de_nodos()->nodos_salientes; nodo != NULL && strcmp(nodo->info.nombre, nodoDestino); nodo = nodo->next) {

	}

	return nodo;
}
nodo_red* buscar_nodo_en_red_por_sock(int connection) {
	nodo_red* nodo = NULL;

	//busco en conexiones entrantes
	for (nodo = red_de_nodos()->nodos_entrantes; nodo != NULL && nodo->info.conexion != connection; nodo = nodo->next) {

	}

	if (nodo != NULL)
		return nodo;
	//si no lo encontro, lo busco en las salientes
	for (nodo = red_de_nodos()->nodos_salientes; nodo != NULL && nodo->info.conexion != connection; nodo = nodo->next) {

	}

	return nodo;
}


nodo_red* buscar_dc() {
	nodo_red* nodo = NULL;

	if (red_de_nodos()->mi_nodo->info.dc)
		return red_de_nodos()->mi_nodo;

	//busco en conexiones entrantes
	for (nodo = red_de_nodos()->nodos_entrantes; nodo != NULL && !nodo->info.dc; nodo = nodo->next) {

	}

	if (nodo != NULL)
		return nodo;
	//si no lo encontro, lo busco en las salientes
	for (nodo = red_de_nodos()->nodos_salientes; nodo != NULL && !nodo->info.dc; nodo = nodo->next) {

	}

	return nodo;
}


typedef struct {
    char* ip;
    char* puerto;
    int time;
} refrescador_params;

pthread_t thread_refrescador;

void* refrescador(void* untyped_params) {
    refrescador_params* params = (refrescador_params*) untyped_params;

    SocketBuilderClass* builder = new SocketBuilderClass();
    builder->ip( params->ip );
    builder->puerto( params->puerto );
    SocketClass<>* sock = (SocketClass<>*) builder->startUnconnected();
    insomnio_header header = {"", TYPE_REFRESH, 0, 0, 0};
    SocketClass<insomnio_header> sock_header(sock->id());
    
    while (1) {
        sleep( params->time );
        debug << "ya paso " << time << " segundos, enviando refresco " << endl;
        sock_header << header;
    }
    
    return NULL;
}


void activar_refresco_de_red_por_intervalo(char* configName, char* ip, char* puerto) {
	info << " activando refresco por intervalo " << endl;
        char* str_time = read_config(configName, "refresh_time");
        if (!str_time) {
            error << " no se pudo determinar el tiempo de intervalo " << endl;
        }
        int time = atoi(str_time);
        if (!time) {
            error << " no se pudo determinar el tiempo de intervalo, valor por defecto 3 " << endl;
            time = 3;
        }
        refrescador_params* params = new refrescador_params;
        params->ip = ip;
        params->puerto = puerto;
        params->time = time;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + 1024);

        pthread_create(&thread_refrescador,NULL, &refrescador, params);
        
    pthread_attr_destroy(&attr);
        
        info << " iniciado REFRESCADOR " << endl;

}


