#ifndef __ansi_webConnector_h
#define __ansi_webConnector_h

#ifdef __cplusplus
extern "C" {
#endif

#include <messages/webserver-buffer.h>

#define XML_SEPARATOR "|"
int web_connect(const char* ip, const char* port);
char* web_request(int sock, const char* msg);
void web_close(int sock);

#ifdef __cplusplus
}
#endif

#endif 