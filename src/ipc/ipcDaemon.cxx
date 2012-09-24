/*
 * ipcDeamon.c
 *
 *  Created on: 23/11/2010
 *      Author: pmarchesi
 */
#include "ipcDaemon.h"
#include <sys/msg.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <util/log_ansi.h>
#include <sys/socket.h>
#include <ipc/socketBuilder.h>
#include "node/daemon/handshake.h"
#define modulo "ipcDeamon"

pthread_mutex_t* mutexSend = NULL;
int sendMessage(int msgid, insomnio_header *header, void* message, size_t size) {
	if (mutexSend == NULL){
		mutexSend = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(mutexSend,NULL);
	}
	pthread_mutex_lock(mutexSend);
	header->length = size;
	int escrito = send(msgid, header, sizeof(insomnio_header), 0);
//	debug(modulo, "Header(%d) Escrito %d de %d\n", header->payload, escrito,sizeof(insomnio_header));
	if (!(message == NULL || size == 0 )){
		escrito += send(msgid, message, size, 0);
//		debug(modulo, "Escrito %d de %d\n", escrito,size);
	}
//	fprintf(stderr,"put %s %d\n", header->id, msgid);
	pthread_mutex_unlock(mutexSend);
	return escrito;
}

int readMessage(int msgid, void* message, size_t size) {
//	fprintf(stderr,"lock %lu %d\n",pthread_self(), msgid);

	int rta = recv(msgid, message, size, 0);
//	debug(modulo, "Leido %d de %d\n", rta,sizeof(insomnio_header));
//	fprintf(stderr, "go rta %d\n" , rta);
	return rta;
}
int conectar_a(char* ip, char* puerto) {
	SocketBuilderClass* builder = new SocketBuilderClass();
	builder->ip(ip);
	builder->puerto(puerto);
//	debug(modulo, "Conectando a %s:%s\n", ip, puerto);
	SocketClass<>* sock = (SocketClass<>*) builder->startConnection();
	int id = -1;
	if (sock != NULL && client_handshake(sock)) {
		id = sock->id();
	}
//	debug(modulo, "Listo HandShake  (%d)\n",sock);
	if (sock != NULL)
		delete sock;
	delete builder;
	return id;
}
