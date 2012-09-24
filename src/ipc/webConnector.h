#ifndef __webConnector_h
#define __webConnector_h

#include "ansi/webConnector.h"
#include "socket.h"

void web_send(SocketClass<web_line> sock, const char* msg);
char* web_recv(SocketClass<web_line> sock);

#endif
