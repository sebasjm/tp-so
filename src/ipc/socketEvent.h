#ifndef __socketEvent_h
#define __socketEvent_h

#include <ipc/socket.h>
#include <ipc/socketGate.h>

using namespace std;

class SocketEventClass {
public:
    SocketEventClass();
    virtual ~SocketEventClass();

    virtual bool onMessageRecv(PointClass<>* sock) = 0;
    virtual void onMessageSent(PointClass<>* sock) = 0;
    virtual void onMessageError(PointClass<>* sock) = 0;
    virtual pair<PointClass<>*, SocketEventClass*> onNewConnection(SocketGateClass* sock) = 0;

};

#endif
