#include "webConnector.h"
#include "../webConnector.h"
#include "../socketBuilder.h"
#include "../socket.h"

#include <string.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

#include <util/log.h>
__init_log(webConector)

int web_connect(const char* _ip, const char* _port) {
	string* ip = new string(_ip);
	string* port = new string(_port);
	SocketBuilderClass* sb = new SocketBuilderClass();
	sb->ip(*ip);
	sb->puerto(*port);
	SocketClass<web_line>* s = (SocketClass<web_line>*) sb->startConnection();
	if (s == NULL)
		return 0;

	int result = s->id();
	delete s;
	delete ip;
	delete port;
	delete sb;
	return result;
}

char* web_request(int s, const char* msg) {
	SocketClass<web_line>* sock = new SocketClass<web_line> (s);
	web_send(*sock, msg);
	//	web_send(*sock, "\n\n");
	char* result = web_recv(*sock);
	delete sock;
	return result;
}

void web_close(int s) {
	SocketClass<web_line>* sock = new SocketClass<web_line> (s);
	sock->stop();
	delete sock;
}

char* web_recv(SocketClass<web_line> sock) {
	char* result = NULL;
	int size_buf = 0;

	web_line buf;
	while (!sock.hasEnded()) {
		sock >> buf;
		int size_msg = buf.str[sizeof(buf.str) - 1] ? sizeof(buf.str) : strlen(buf.str);
		if (!result) {
			result = (char*) malloc(size_msg + size_buf);
		} else {
			char* resultTemp = (char*) realloc(result, size_msg + size_buf);
			if (resultTemp == NULL) {
				fprintf(stderr, "ERROR MAS QUE GRAVE.. NO ME DA MEMORY, NO SEAS RATA Y COMPRA MAS!!\n");
			}
			result = resultTemp;
		}
		memcpy(&result[size_buf], &(buf.str), size_msg);
		size_buf += size_msg;
	}
	char* resultTemp = (char*) realloc(result, size_buf +1 );
	if (resultTemp == NULL) {
		fprintf(stderr, "ERROR MAS QUE GRAVE.. NO ME DA MEMORY, NO SEAS RATA Y COMPRA MAS!!\n");
	}
	result = resultTemp;
	result[size_buf] = '\0';
	return result;
}
void web_send(SocketClass<web_line> sock, const char* msg) {
	int limit = strlen(msg);
	web_line line;
	int index = 0;
	while (index < limit) {
		strncpy(line.str, &msg[index], sizeof(web_line));
		if (limit - index < sizeof(web_line)) {
			line.str[limit - index] = '\0';
		}
		sock << line;
		index += sizeof(web_line);
	}
}
