#include "socketGate.h"
#include "socket.h"
#include <fstream>
#include <string>

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <so_agnostic.h>

using namespace std;

#include <util/log.h>
__init_log(socketGate)

SocketGateClass::SocketGateClass(const int id) {
    this->_id = id;
}

SocketGateClass::~SocketGateClass() {

}

SocketClass<>* SocketGateClass::acceptConnection(){
    sockaddr_in their_addr;
    #ifdef __CHANCHADAS_MARCA_WINDOWS
    int sin_size = sizeof their_addr;
    #else
    socklen_t sin_size = sizeof their_addr;
    #endif
    
    int sockfd = accept( this->_id, (struct sockaddr *)&their_addr, &sin_size);

    if (sockfd == -1) {
        error << " aceptando la conexion, socket " << sockfd << endl;
    }

    return new SocketClass<>(sockfd);
}

const int SocketGateClass::id() {
    return this->_id;
}

SocketGateClass::operator int() {
    return this->_id;
}

