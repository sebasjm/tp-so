#include <stdlib.h>
#include <util/config.h>
#include "networkManager.h"
#include <iostream>
#include <messages/daemon-daemon.h>
#include <ipc/socket.h>
#include <node/daemon/common.h>
#include <node/daemon/netDiscovery.h>
#include <messages/teclado-consola.h>

using namespace std;

#include <util/log.h>

__init_log(daemon_commands)

void resolver_refresh( char* nodoName ) {
    debug << " refrescado la red" << endl;
    insomnio_header header = { "", TYPE_PING, TTL, 1, 1 };
    sprintf(header.id, "%s", nodoName);
    
    //solo mando ping de refresco a la red si soy el DC
    //como todos van a responder con pong, todos les llegaria la info
    //actualizada
    if ( red_de_nodos()->mi_nodo->info.dc ) {
    	debug << " soy el dc, asi que hago ping para que todos se actualicen" << endl;
    	broadcast(nodoName, red_de_nodos()->mi_nodo->info, header);
    } else {
        if ( buscar_dc() == NULL || buscar_dc()->info.tiempo_sin_pong != TIEMPO_SIN_PONG ) {
            debug << " parece que el dc no responde, hago pingo antes de que todo colapse " << endl;
            broadcast(nodoName, red_de_nodos()->mi_nodo->info, header);
        }
    }
    
    
    //verificar de eliminar los que no respondieron por un tiempo
    nodo_red* i;
    nodo_red* ant = NULL;
    debug << " reviso la lista para ver si tengo q sacar algun nodo" << endl;
    for(i = red_de_nodos()->nodos_entrantes; i != NULL; i = i->next) {
        i->info.tiempo_sin_pong--;
        if ( i->info.tiempo_sin_pong == 0) {
            info << " el nodo " << i->info.nombre << " no respondio por " << TIEMPO_SIN_PONG << " intentos " << endl;
            finalizo_nodo(&i->info);
            if ( ant == NULL ) {
                red_de_nodos()->nodos_entrantes = i->next;
            } else {
                ant->next = i->next;
            }
        }
        ant = i;
    }

    ant = NULL;
    for(i = red_de_nodos()->nodos_salientes; i != NULL; i = i->next) {
        i->info.tiempo_sin_pong--;
        if ( i->info.tiempo_sin_pong == 0) {
            info << " el nodo " << i->info.nombre << " no respondio por " << TIEMPO_SIN_PONG << " intentos " << endl;
            finalizo_nodo(&i->info);
            if ( ant == NULL ) {
                red_de_nodos()->nodos_salientes = i->next;
            } else {
                ant->next = i->next;
            }
        }
        ant = i;
    }
}

void resolver_display(int conexion) {
    info_nodo listo_el_pollo = {"listo","listo","listo","listo",0,0,0,0,0};
    nombre_nodo pelada_la_gallina = {"listo"};
    
    char* nombre_saliente;
    nodo_red* ptr;
    
    SocketClass<info_nodo> sock_info( conexion );
    SocketClass<nombre_nodo> sock_nombre( conexion );

    sock_info << red_de_nodos()->mi_nodo->info;

    for (ptr = red_de_nodos()->nodos_entrantes; ptr != NULL; ptr = ptr->next) {
        sock_info << ptr->info;
    }

    sock_info << listo_el_pollo;

    for (ptr = red_de_nodos()->nodos_salientes; ptr != NULL; ptr = ptr->next) {
        sock_info << ptr->info;
    }

    sock_info << listo_el_pollo;

    nombre_saliente = red_de_nodos()->mi_nodo->info.nombre_saliente;
    ptr = buscar_nodo_por_nombre(nombre_saliente, red_de_nodos()->nodos_salientes);
    while (ptr != NULL) {
        nombre_nodo msg;
        sprintf(msg.id,"%s",ptr->info.nombre);
        sock_nombre << msg;
        ptr = buscar_nodo_por_nombre(ptr->info.nombre_saliente, red_de_nodos()->nodos_salientes);
    };

    sock_nombre << pelada_la_gallina;
}

void resolver_stop() {
    nodo_red* ptr;
    for (ptr = red_de_nodos()->nodos_entrantes; ptr != NULL; ptr = ptr->next) {
        if (ptr->info.distancia == 1) {
            SocketClass<> sock(ptr->info.conexion);
            sock.stop();
        }
    }
    for (ptr = red_de_nodos()->nodos_salientes; ptr != NULL; ptr = ptr->next) {
        if (ptr->info.distancia == 1) {
            SocketClass<> sock(ptr->info.conexion);
            sock.stop();
        }
    }
}
