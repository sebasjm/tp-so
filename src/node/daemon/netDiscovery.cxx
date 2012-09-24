#include "netDiscovery.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>

#include <ipc/socket.h>
#include <ipc/socketBuilder.h>
#include "handshake.h"
#include <util/config.h>
#include "networkManager.h"

#include <node/daemon/common.h>

#include <util/log.h>

__init_log(netDiscovery)

using namespace std;

nodo_red* buscame_algun_nodo_para_conectarme_a_partir_de(nodo_red* nodo_base, nodo_red* ya_buscados);
nodo_red* probar_con_los_entrantes_a_partir_de(nodo_red* nodo_base, nodo_red* ya_buscados);
nodo_red* prox_nodo_saliente(char* nombre, nodo_red* last_found);
void pasar_nodos_salientes_a_entrantes();

void reenviar_ping(char* nodoName, info_nodo info_ping, char* nombreDelQueLlama, insomnio_header header) {
	nodo_red* i;

	//actualizo ttl
	header.hops++;
	header.ttl--;
	//verifico ttl
	if (header.ttl == 0)
		return;
	info_ping.distancia++;

	for (i = red_de_nodos()->nodos_entrantes; i != NULL; i = i->next) {
		if (i->info.distancia != 1)
			continue;
		if (!strcmp(i->info.nombre, nombreDelQueLlama))
			continue;

		enviar_msg_ping(i->info.nombre, info_ping, header);
	}

	for (i = red_de_nodos()->nodos_salientes; i != NULL; i = i->next) {
		if (i->info.distancia != 1)
			continue;
		if (!strcmp(i->info.nombre, nombreDelQueLlama))
			continue;

		enviar_msg_ping(i->info.nombre, info_ping, header);
	}

}

void broadcast(char* nodoName, info_nodo mi_info, insomnio_header pong) {
	nodo_red* i;

	for (i = red_de_nodos()->nodos_entrantes; i != NULL; i = i->next) {
		if (i->info.distancia != 1)
			continue;
		enviar_msg_pong(i->info.nombre, mi_info, pong);

	}

	for (i = red_de_nodos()->nodos_salientes; i != NULL; i = i->next) {
		if (i->info.distancia != 1)
			continue;
		enviar_msg_pong(i->info.nombre, mi_info, pong);
	}
}

void resolver_msg_ping(char* configName, char* nodoName, char* nombreDelNodoQueLlama, info_nodo infonodo, insomnio_header header) {
	nodo_red* nodo = buscar_nodo_en_red(infonodo.nombre);
	int nuevoNodo = 0;
	debug << " reciviendo PING de " << header.id << " desde " << nombreDelNodoQueLlama << " host" << header.hops << endl;

	if (nodo == NULL) {
		info << " Nuevo nodo "<< infonodo.nombre << "("<< header.hops <<") via PING de " << nombreDelNodoQueLlama << endl;
		nodo = new nodo_red;
		nodo->next = NULL;
		nodo->info.distancia = 999;
		nuevoNodo = 1;
		agregar_al_final(nodo, viene_por_saliente(nombreDelNodoQueLlama, infonodo) ? red_de_nodos()->nodos_salientes : red_de_nodos()->nodos_entrantes);
                nuevo_nodo(&infonodo);
	}

	nodo->info.dc = infonodo.dc;
	nodo->info.memoria = infonodo.memoria;
	nodo->info.disco = infonodo.disco;
        nodo->info.conexion = header.hops==1 ? infonodo.conexion : 0;
	sprintf(nodo->info.ip, "%s", infonodo.ip);
	sprintf(nodo->info.puerto, "%s", infonodo.puerto);
	sprintf(nodo->info.nombre, "%s", infonodo.nombre);
	sprintf(nodo->info.nombre_saliente, "%s", infonodo.nombre_saliente);
	nodo->info.distancia = infonodo.distancia;
	nodo->info.tiempo_sin_pong = TIEMPO_SIN_PONG;
//	if (nuevoNodo)
		

	reenviar_ping(nodoName, infonodo, nombreDelNodoQueLlama, header);

	insomnio_header pong = { "", TYPE_PONG, TTL, 1, 1 };
	sprintf(pong.id, "%s", nodoName);

	broadcast(nodoName, red_de_nodos()->mi_nodo->info, pong);

}

void resolver_msg_pong(char* nodoName, char* nombreDelNodoQueLlama, info_nodo infonodo, insomnio_header header) {
	nodo_red* nodo = buscar_nodo_en_red(infonodo.nombre);

	debug << " recibiendo PONG de " << header.id << " desde " << nombreDelNodoQueLlama << " host" << header.hops << endl;

	if (nodo == NULL) {
		info << " Nuevo nodo "<< infonodo.nombre << "("<< header.hops <<") via PONG de " << nombreDelNodoQueLlama << endl;
		nodo = new nodo_red;
		nodo->next = NULL;
		nodo->info.distancia = 999;

		agregar_al_final(nodo, viene_por_saliente(nombreDelNodoQueLlama, infonodo) ? red_de_nodos()->nodos_salientes : red_de_nodos()->nodos_entrantes);
                nuevo_nodo(&infonodo);
	}

	nodo->info.dc = infonodo.dc;
	nodo->info.memoria = infonodo.memoria;
	nodo->info.disco = infonodo.disco;
	nodo->info.conexion = header.hops==1 ? infonodo.conexion : 0;
	sprintf(nodo->info.ip, "%s", infonodo.ip);
	sprintf(nodo->info.puerto, "%s", infonodo.puerto);
	sprintf(nodo->info.nombre, "%s", infonodo.nombre);
	sprintf(nodo->info.nombre_saliente, "%s", infonodo.nombre_saliente);
	nodo->info.distancia = infonodo.distancia;
//	debug << "PONG2 BORRAR ESTE LOG " << nodo->info.nombre << ":" << nodo->info.conexion <<" Dist"<< nodo->info.distancia  << endl;
	nodo->info.tiempo_sin_pong = TIEMPO_SIN_PONG;

	reenviar_ping(nodoName, infonodo, nombreDelNodoQueLlama, header);
}

int enviar_ping_nuevo(char* nodoName, char* configName, char* la_ip, char* el_puerto) {
	insomnio_header header = { "", TYPE_PING, TTL, 1, 1 };
	sprintf(header.id, "%s", nodoName);

	nodo_red* nodo_saliente = new nodo_red;
	nodo_saliente->info.conexion = 0;
	nodo_saliente->info.dc = false;
	nodo_saliente->info.distancia = 1;
	sprintf(nodo_saliente->info.ip, "%s", la_ip);
	sprintf(nodo_saliente->info.puerto, "%s", el_puerto);
	sprintf(nodo_saliente->info.nombre, "%s", "saliente");
	nodo_saliente->next = NULL;
	red_de_nodos()->nodos_salientes = nodo_saliente;

	int result = 0;
	if (enviar_msg_ping("saliente", red_de_nodos()->mi_nodo->info, header)) {
		result = nodo_saliente->info.conexion;
	}

	delete nodo_saliente;
	red_de_nodos()->nodos_salientes = NULL;

	return result;
}

int reconectar_a_otro_saliente(nodo_red* nodo_caido) {
    nodo_red* prox_nodo = buscame_algun_nodo_para_conectarme_a_partir_de(nodo_caido,NULL);

    if (prox_nodo != NULL) {
        info << " nuevo saliente " << prox_nodo->info.nombre << endl;
            //limpiar lista de saliente
            memcpy(red_de_nodos()->mi_nodo->info.nombre_saliente, prox_nodo->info.nombre, sizeof(red_de_nodos()->mi_nodo->info.nombre));

            insomnio_header header = {"", TYPE_PING, TTL, 1, 1};
            sprintf(header.id, "%s", red_de_nodos()->mi_nodo->info.nombre);
            enviar_msg_ping( prox_nodo->info.nombre, red_de_nodos()->mi_nodo->info, header);
            return prox_nodo->info.conexion;
    } else {
            info << " no encontre nadie con quien conectarme " << endl;
            memset(red_de_nodos()->mi_nodo->info.nombre_saliente, 0, sizeof(red_de_nodos()->mi_nodo->info.nombre));
            // como me di cuenta q no me tengo q conectar a nadie
            // todos van a queda atras mio
            pasar_nodos_salientes_a_entrantes();
    }
    return 0;
}

void pasar_nodos_salientes_a_entrantes() {
	nodo_red* ptr = NULL;
	for (ptr = red_de_nodos()->nodos_entrantes; ptr != NULL && ptr->next != NULL; ptr = ptr->next) {
	}
	if (ptr == NULL) {
		red_de_nodos()->nodos_entrantes = red_de_nodos()->nodos_salientes;
	} else {
		ptr->next = red_de_nodos()->nodos_salientes;
	}
	red_de_nodos()->nodos_salientes = NULL;
}


nodo_red* prox_nodo_saliente(char* nombre, nodo_red* last_found) {
	nodo_red* nodo;
	char* menor = NULL;
	if (!nombre)
		return NULL;

//        debug << " buscando prox nodo con saliente " << nombre << endl;

	for (nodo = red_de_nodos()->nodos_salientes; nodo != NULL; nodo = nodo->next) {
		//        debug << nodoName << " menor " << (menor==NULL?" - ":menor) << " -> " << nodo->info.nombre << endl;
		//tiene q ser un nodo que tenga a 'nombre' como saliente
		if (nodo->info.nombre_saliente == NULL || strcmp(nodo->info.nombre_saliente, nombre)) {
//                        debug <<  " el nodo " << nodo->info.nombre << " no tiene el saliente que busco " << nodo->info.nombre_saliente << endl;
			continue;
		}
		//tiene q ser un nombre menor al mio
		if (strcmp(nodo->info.nombre, red_de_nodos()->mi_nodo->info.nombre ) >= 0) {
//                        debug << " el nombre es mayor al mio " << nodo->info.nombre << endl;
			continue;
		}
		//tiene q ser mayor que el ultimo encontrado
		if (last_found != NULL && strcmp(nodo->info.nombre, last_found->info.nombre) <= 0) {
//                        debug << " es menor al ultimo que busque " << nodo->info.nombre << " | " << last_found->info.nombre << endl;
			continue;
		}
		//el primero siempre es el menor
		if (menor == NULL)
			menor = nodo->info.nombre;
		if (strcmp(nodo->info.nombre, menor) < 0)
			menor = nodo->info.nombre;
	}
//	    debug << " busqueda terminada " << (menor?menor:"null") << endl;
	return buscar_nodo_por_nombre(menor, red_de_nodos()->nodos_salientes);
}

nodo_red* probar_con_los_entrantes_a_partir_de(nodo_red* nodo_base, nodo_red* ya_buscados) {
    if (nodo_base == NULL)
            return NULL;

    for (nodo_red* nodo = prox_nodo_saliente(nodo_base->info.nombre, NULL); nodo != NULL; nodo = prox_nodo_saliente(nodo_base->info.nombre, nodo)) {
        if ( ya_fue_buscado(nodo, ya_buscados) ) continue;

        debug << " entrante -> " << nodo->info.nombre << endl;

        int conexion = conectar_a_nodo(nodo->info.nombre);
        if (conexion) {
            nodo->info.conexion = conexion;
            debug << " me conecte " << endl;
            return nodo;
        } else {
            debug << " no me pude conectar con " << nodo->info.nombre << endl;
        }

    }
    // si no me pude conectar con ninguno, pruebo con los entrantes de los entrantes
    debug << "no me pude conectar a ningun entrante, pruebo con los entrantes de ellos " << endl;
    for (nodo_red* nodo = prox_nodo_saliente(nodo_base->info.nombre, NULL); nodo != NULL; nodo = prox_nodo_saliente(nodo_base->info.nombre, nodo)) {
        if ( ya_fue_buscado(nodo, ya_buscados) ) continue;

        debug << " entrante -> " << nodo->info.nombre << endl;

        nodo_red* nodo_entrante = probar_con_los_entrantes_a_partir_de(nodo, ya_buscados);

        if (nodo_entrante != NULL) {
            return nodo_entrante;
        }
    }
    return NULL;

}

nodo_red* buscame_algun_nodo_para_conectarme_a_partir_de(nodo_red* nodo_base, nodo_red* ya_buscados) {
	if (nodo_base == NULL || ya_fue_buscado(nodo_base, ya_buscados))
		return NULL;

        debug << " base " << nodo_base->info.nombre << " con saliente " << nodo_base->info.nombre_saliente << endl;

	nodo_red* tmp = new nodo_red;
	tmp->info = nodo_base->info;
	tmp->next = ya_buscados;
	ya_buscados = tmp;

        nodo_red* prox = buscar_nodo_en_red(nodo_base->info.nombre_saliente);
        debug << " probando conexion con " << nodo_base->info.nombre_saliente << endl;
        if (prox != NULL) {
            prox->info.conexion = 0;
            int conexion = conectar_a_nodo(prox->info.nombre);
            if (conexion) {
                prox->info.conexion = conexion;
                debug << " me conecte " << endl;
                return prox;
            } else {
                debug << " no me pude conectar, busco otro a partir de " << prox->info.nombre << endl;
                //			nodo_red* nodo_saliente = buscar_nodo_por_nombre(prox->info.nombre, red_de_nodos()->nodos_salientes);
                return buscame_algun_nodo_para_conectarme_a_partir_de(prox, ya_buscados);
            }
        }

        debug << " probando con los nodos conectados a " << nodo_base->info.nombre << endl;
	return probar_con_los_entrantes_a_partir_de(nodo_base,ya_buscados);
}
