#include "conectorCliente.h"
#include "ipc/socket.h"
#include "ipc/socketEvent.h"
#include "ipc/point.h"
#include "resolvers.h"
#include <string.h>
#include <algorithm>
#include <regex.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <so_agnostic.h>
#ifdef __CHANCHADAS_MARCA_WINDOWS
#include <ipc/point.cxx>
#include <ipc/ansi/webConnector.cxx>
#endif

#ifndef __CHANCHADAS_MARCA_WINDOWS
#include <util/log.h>
__init_log(conectorCliente)
#endif

using namespace std;

volatile endpoints_info endpoints[] = {
    { "", &resolver_root } ,
    { "webserver/register", &resolver_register } ,
    { "webserver/status", &resolver_status } ,
    { "webserver/readyQueue", &resolver_ready } ,
    { "webserver/doneQueue", &resolver_done } ,
    { "webserver/errorQueue", &resolver_error } ,
    { "webserver/result", &resolver_result } ,
    { NULL, &resolver_server_404 }
};

#define EMPTY_STRING ""

char* take_param(int id, char* request, regmatch_t matchs[]) {
    int len = strlen(request);
    
    int cut = matchs[id+1].rm_eo != -1 ? matchs[id+1].rm_so : len;
    char save = request[cut];
    request[cut] = '\0';

    len = strlen(&request[matchs[id].rm_so]);
    char* result = new char[len+1];
    strcpy(result,&request[matchs[id].rm_so]);

    request[cut] = save;
    return result;
}

vector<string> split(const string& s, const string& delim, const bool keep_empty = true) {
    vector<string> result;
    if (delim.empty()) {
        result.push_back(s);
        return result;
    }
    string::const_iterator substart = s.begin(), subend;
    while (true) {
        subend = search(substart, s.end(), delim.begin(), delim.end());
        string temp(substart, subend);
        if (keep_empty || !temp.empty()) {
            result.push_back(temp);
        }
        if (subend == s.end()) {
            break;
        }
        substart = subend + delim.size();
    }
    return result;
}

url_info get_url_info(char* request) {
    regex_t regex;
    int reti;
    regmatch_t machs[10];
    reti = regcomp(&regex, "^GET /([a-z/]*)(.do(\\?{0,1}))(.*)( )HTTP.*$", REG_ICASE|REG_EXTENDED);
    reti = regexec(&regex, request, 10, machs, 0);
    if (reti) {
        url_info null;
        null.pagina = NULL;
        return null;
    } 

    url_info d;
    d.pagina = take_param(1,request,machs);
    d.request_params = split(take_param(4,request,machs),"&");
    return d;
}

ConectorCliente::ConectorCliente(list<JobClass*>* ready_q) {
    this->_ready_queue = ready_q;
}


bool ConectorCliente::onMessageRecv(PointClass<>* conector){
    SocketClass<web_line> sock ( *conector );
    web_line mensaje;

    sock >> mensaje;

//	debug << "conexion mensaje " << mensaje.str << " closed? " << sock.isClosed() << endl;

    if (sock.isClosed()) return true;

    mensaje.str[sizeof(mensaje.str)] = '\0';
    int d = 0;
    while (mensaje.str[++d]!='\n');
    mensaje.str[d]='\0';


    if ( strncmp("GET",mensaje.str,3) != 0 ) return true;
    url_info request_info = get_url_info( mensaje.str );

    sockaddr_in asd;
    #ifdef __CHANCHADAS_MARCA_WINDOWS
    int number = sizeof(sockaddr_in);
    #else
    socklen_t number = sizeof(sockaddr_in);
    #endif
    getsockname(sock, (struct sockaddr *)&asd ,&number);
    strcpy(request_info.ip, inet_ntoa(asd.sin_addr));
    request_info.ready_queue = this->_ready_queue;

    resolver_server_header(sock,request_info);

    int i = 0;
    for (; request_info.pagina != NULL && endpoints[i].name != NULL; i++) {
        if ( strcmp( request_info.pagina, endpoints[i].name ) == 0 ) {
            endpoints[i].resolver( sock, request_info );
            return true;
        }
    }
    
    endpoints[i].resolver( sock, request_info ); /*Si no encontro, responde con el ultimo*/
    return true;
}

pair<PointClass<>*, SocketEventClass*> ConectorCliente::onNewConnection(SocketGateClass* listener){
    SocketClass<>* sock = listener->acceptConnection();
    return pair<PointClass<>*, SocketEventClass*>( sock, this );
}

void ConectorCliente::onMessageSent(PointClass<>* sock){
    
}

void ConectorCliente::onMessageError(PointClass<>* sock){

}



void resolver_root(SocketClass<web_line> sock) {

}

