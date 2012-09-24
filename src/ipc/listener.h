#ifndef __listener_h
#define __listener_h

#include <ipc/ipcstream.h>
#include <ipc/point.h>
#include <ipc/socketGate.h>
#include <ipc/socketEvent.h>
#include <map>
#include <list>
#include <sys/select.h>

typedef map<int, pair<PointClass<>*, SocketEventClass*> > socketEventMap;
typedef map<int, pair<SocketGateClass*, SocketEventClass*> > socketGateEventMap;

class Listener {
public:
    Listener();
    ~Listener();
    template <typename type> void add(PointClass<type>* socket, SocketEventClass* event );
    void add(SocketGateClass* socket, SocketEventClass* event );
    void loop();
    void stop();
private:
    fd_set* master_fd_set;

    fd_set* write_fd_set;
    fd_set* read_fd_set;
    fd_set* error_fd_set;

    socketEventMap* sockets;
    socketGateEventMap* socketsGate;
    list<int>* fileDescriptors;

    void waitUntilReady();
    bool _stop;

};

#endif