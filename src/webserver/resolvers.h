#ifndef __webserver_resolvers_h
#define __webserver_resolvers_h

#ifndef NULL
    #define NULL 0l
#endif

#include <ipc/socket.h>
#include <messages/webserver-buffer.h>
#include <vector>
#include <list>
#include <string>
#include <model/job.h>

struct url_info {
    char ip[16];
    char* pagina;
    vector<string> request_params;
    list<JobClass*>* ready_queue;
};

typedef void (*resolver_func)(SocketClass<web_line> sock, url_info info);

struct endpoints_info {
    const char* name;
    const resolver_func resolver;
};

void resolver_root     (SocketClass<web_line> sock, url_info info);
void resolver_register (SocketClass<web_line> sock, url_info info);
void resolver_status   (SocketClass<web_line> sock, url_info info);

void resolver_ready    (SocketClass<web_line> sock, url_info info);
void resolver_done     (SocketClass<web_line> sock, url_info info);
void resolver_error    (SocketClass<web_line> sock, url_info info);
void resolver_result   (SocketClass<web_line> sock, url_info info);

void resolver_server_404    (SocketClass<web_line> sock, url_info info);
void resolver_server_header (SocketClass<web_line> sock, url_info info);
#endif
