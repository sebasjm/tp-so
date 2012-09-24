/*
 * ipcDeamon.h
 *
 *  Created on: 23/11/2010
 *      Author: pmarchesi
 */

#ifndef IPCDEAMON_H_
#define IPCDEAMON_H_
#include "messages/daemon-daemon.h"
#include <stddef.h>

int sendMessage(int msgid, insomnio_header *header, void* message, size_t size);
int readMessage(int msgid, void* message, size_t size);

int conectar_a(char* ip, char* puerto);

#endif /* IPCDEAMON_H_ */
