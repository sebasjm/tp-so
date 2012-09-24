#ifndef __webserver_conector_cliente_h
#define __webserver_conector_cliente_h

#include <ipc/point.h>
#include <ipc/socketEvent.h>
#include <ipc/socket.h>
#include <messages/webserver-buffer.h>
#include <list>
#include <string>
#include <model/job.h>

class ConectorCliente: public SocketEventClass {

public:
    ConectorCliente(list<JobClass*>* ready_q);

    virtual bool onMessageRecv(PointClass<>* conector);
    virtual pair<PointClass<>*, SocketEventClass*> onNewConnection(SocketGateClass* listener);
    virtual void onMessageSent(PointClass<>* sock);
    virtual void onMessageError(PointClass<>* sock);
private:
    list<JobClass*>* _ready_queue;
};


#endif
