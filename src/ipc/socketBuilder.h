#ifndef __socketBuilder_h
#define __socketBuilder_h

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "socket.h"
#include "socketGate.h"

using namespace std;

class SocketBuilderClass {
public:
    SocketBuilderClass();

    SocketBuilderClass ip(string ip);
    SocketBuilderClass puerto(string puerto);
    SocketBuilderClass puerto(int puerto);
    
    SocketGateClass* createBind();
    SocketClass<>* createUnbinded();
    SocketClass<>* startConnection();
    SocketClass<>* startUnconnected();

private:
    sockaddr_in addr;
};

#endif
