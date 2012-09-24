#include "socketBuilder.h"
#include "socket.h"
#include <messages/interface-daemon.h>

#include <fstream>
#include <string.h>
#include <string>

#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace std;

#include <util/log.h>
__init_log(socketBuilder)


SocketBuilderClass::SocketBuilderClass() {
    this->addr.sin_family = AF_INET;
    this->addr.sin_addr.s_addr = INADDR_ANY;
}

SocketBuilderClass SocketBuilderClass::ip(string ip) {
    this->addr.sin_addr.s_addr = inet_addr(ip.data());
    memset(this->addr.sin_zero, '\0', sizeof this->addr.sin_zero);
//    hostent *host = gethostbyname(ip.data());
//    this->addr.sin_addr.s_addr = ((in_addr*)(host->h_addr))->s_addr;
    return *this;
}

SocketBuilderClass SocketBuilderClass::puerto(string puerto) {
    this->puerto(
        atoi( puerto.data() )
    );
    return *this;
}

SocketBuilderClass SocketBuilderClass::puerto(int puerto) {
    this->addr.sin_port = htons( puerto );
    return *this;
}

#define ip_y_puerto "[" << inet_ntoa(this->addr.sin_addr) << ":" << ntohs(this->addr.sin_port) << "]"

/********************************
 * TCP
 ********************************/

#define ERROR_SOCKET "no se pudo crear el socket " << ip_y_puerto
#define ERROR_BIND "no se pudo hacer bind " << ip_y_puerto << ", espero 2 segundos y vuelvo a intentar"
#define ERROR_LISTEN "no se pudo hacer listen con sock " << sockfd << ip_y_puerto
#define ERROR_CONNECT "no me pude conectar con " << ip_y_puerto


SocketGateClass* SocketBuilderClass::createBind(){
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        error << "TCP " << ERROR_SOCKET << endl;
        return NULL;
    }

    while( bind(sockfd, (struct sockaddr *)&this->addr, sizeof this->addr) == -1) {
        error << "TCP " << ERROR_BIND << endl;
        sleep(2);
    }

    if (listen(sockfd, 10) == -1) {
        error << "TCP " << ERROR_LISTEN << endl;
        close(sockfd);
        return NULL;
    }
    debug << "Escuchando " << ip_y_puerto << " (" << sockfd << ")" << endl;
    return new SocketGateClass(sockfd);
}

SocketClass<>* SocketBuilderClass::startConnection(){
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        error << "TCP " << ERROR_SOCKET << endl;
        return NULL;
    }
    if (connect(sockfd, (sockaddr*)&this->addr, sizeof(this->addr)) == -1) {
        error << "TCP " << ERROR_CONNECT << endl;
        close(sockfd);
        return NULL;
    }
    debug << "Conectado con " << ip_y_puerto << " (" << sockfd << ")" << endl;
    return new SocketClass<>(sockfd);
}

/********************************
 * UDP
 ********************************/

SocketClass<>* SocketBuilderClass::createUnbinded(){
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error << "UDP " << ERROR_SOCKET << endl;
        return NULL;
    }

    while( bind(sockfd, (struct sockaddr *)&this->addr, sizeof this->addr) == -1) {
        error << "UDP " << ERROR_BIND << endl;
        sleep(2);
    }

    debug << "Escuchando UDP (" << sockfd<< ")" << ip_y_puerto << endl;
    return new SocketClass<>(sockfd);
}

SocketClass<>* SocketBuilderClass::startUnconnected(){
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error << "UDP " << ERROR_SOCKET << endl;
        return NULL;
    }
    if (connect(sockfd, (sockaddr*)&this->addr, sizeof(this->addr)) == -1) {
        error << "UDP " << ERROR_CONNECT << endl;
        close(sockfd);
        return NULL;
    }
    return new SocketClass<>(sockfd);
}




