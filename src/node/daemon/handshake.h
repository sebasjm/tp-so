#ifndef __handshake_h
#define __handshake_h

#define CLIENT_INIT_MSG "INSOMNIO CONNECT/1.0\n\n"
#define SERVER_RESPONSE_MSG "INSOMNIO OK\n\n"

#include <ipc/socket.h>

bool client_handshake(SocketClass<>* sock);
bool server_handshake(SocketClass<>* sock);

#endif