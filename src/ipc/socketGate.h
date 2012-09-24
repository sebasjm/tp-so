#ifndef __socketGate_h
#define __socketGate_h

#include <fstream>
#include <string>
#include <sys/ipc.h>

#include "point.h"
#include <ipc/socket.h>

using namespace std;

class SocketGateClass {
public:
    SocketGateClass(const int id);
    ~SocketGateClass();
    
    operator int();
    const int id();

    SocketClass<>* acceptConnection();
private:
    int _id;
};

#endif
