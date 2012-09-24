
#include "listener.h"

#include <so_agnostic.h>

#ifdef __CHANCHADAS_MARCA_WINDOWS
#include "point.cxx"
#endif

#include <messages/teclado-consola.h>

#include <sys/select.h>
#include <sys/socket.h>

#include <unistd.h>
#include <iostream>
#include <map>
#include <list>
#include <string.h>

using namespace std;

#include <util/log.h>
__init_log(listener)


Listener::Listener() {
    this->master_fd_set = new fd_set;
    this->read_fd_set = new fd_set;
    this->write_fd_set = new fd_set;
    this->error_fd_set = new fd_set;

    FD_ZERO( this->master_fd_set );
    FD_ZERO( this->read_fd_set );
    FD_ZERO( this->write_fd_set );
    FD_ZERO( this->error_fd_set );
    
    this->sockets = new socketEventMap();
    this->socketsGate = new socketGateEventMap();
    this->fileDescriptors = new list<int>();
    this->_stop = false;
}

Listener::~Listener() {
    for (list<int>::iterator i = fileDescriptors->begin(); i != fileDescriptors->end(); i++) {
        debug << "cerradno " << *i << endl;
        shutdown( *i, 2 );
        close( *i );
    }

    delete this->sockets;
    delete this->socketsGate;
    delete this->fileDescriptors;
}

template <typename type>
void Listener::add(PointClass<type>* socket, SocketEventClass* event ) {
    FD_SET( socket->id(), this->master_fd_set );
    
    debug << " Agregando manualmente " << socket->id() << " size " << this->fileDescriptors->size() << endl;;
    this->sockets->insert(
        pair<int, pair<PointClass<>*, SocketEventClass*> >(
            socket->id(),
            pair<PointClass<>*, SocketEventClass*>( (PointClass<>*)socket, event)
        )
    );
    
    this->fileDescriptors->push_front(*socket);
    this->fileDescriptors->sort();
}

void Listener::add(SocketGateClass* socket, SocketEventClass* event ) {
    FD_SET( socket->id(), this->master_fd_set );

    debug << " Agregando manualmente " << socket->id() << " size " << this->fileDescriptors->size() << endl;;
    this->socketsGate->insert(
        pair<int, pair<SocketGateClass*, SocketEventClass*> >(
            socket->id(),
            pair<SocketGateClass*, SocketEventClass*>(socket, event)
        )
    );
    this->fileDescriptors->push_front(*socket);
    this->fileDescriptors->sort();
}

void Listener::stop() {
    this->_stop = true;
}

void Listener::loop() {
    int socket;
    bool deleteSocket;
    pair<PointClass<>*,SocketEventClass*> info;
    pair<SocketGateClass*,SocketEventClass*> infoGate;
    list<pair<PointClass<>*,SocketEventClass*> > newConnections;
    
    do {
        list<pair<PointClass<>*,SocketEventClass*> >::iterator it;
        for(it = newConnections.begin(); it != newConnections.end() ; it++ ) {
//            cout << " added " << endl;
            this->add(it->first,it->second);
        }
        newConnections.clear();

//        cerr << "wating!" << endl;

        this->waitUntilReady();

//        cerr << "ready!" << endl;
        
        for(socket = 0; socket <= this->fileDescriptors->back(); socket++) {
            //Si se detecto una lectura
            if (FD_ISSET(socket, this->read_fd_set)) {
//                cerr << "   "  << "read ready " << socket << endl;

                if ( this->sockets->count( socket ) != 0 ) {
                    info = this->sockets->find( socket )->second;
                    deleteSocket = info.second->onMessageRecv( info.first );

                    if ( deleteSocket ) {
                        FD_CLR( socket, this->master_fd_set );

                        this->sockets->erase( socket );
                        this->fileDescriptors->remove( socket );

                        shutdown( socket, 2 );
                        close( socket );
                        debug << " Una conexion menos " << info.first->id() << " size " << this->fileDescriptors->size() << endl;
                    }
                }

                if ( this->socketsGate->count( socket ) != 0 ) {
                    infoGate = this->socketsGate->find( socket )->second;
                    info = infoGate.second->onNewConnection( infoGate.first );

                    if ( info.first != NULL && info.second != NULL ) {
                        newConnections.push_front( pair<PointClass<>*,SocketEventClass*>(info.first , info.second) );
                        debug << " Agregando nueva conexion " << info.first->id() << " size " << this->fileDescriptors->size() << endl;
                    } else {
                        FD_CLR( socket, this->master_fd_set );

                        this->socketsGate->erase( socket );
                        this->fileDescriptors->remove( socket );

                        shutdown( socket, 2 );
                        close ( socket );
                    }
                }
            }
            //Si se detecto una escritura
//            if (FD_ISSET(socket, this->write_fd_set)) {
//                error << "Evento listo para escritura" << endl;
//                cerr << "   "  << "write ready " << socket << endl;
    //            info = this->sockets->find( socket )->second;
    //            info.second->onMessageSent( *info.first );
//            }
            //Si se detecto un error
            if (FD_ISSET(socket, this->error_fd_set)) {
                error << "Evento Error - Eliminando socket de la lista "<< socket << endl;
                shutdown( socket, 2 );
                close ( socket );
                
                FD_CLR( socket, this->master_fd_set );
                if ( this->sockets->count( socket ) != 0 ) {
                    this->sockets->erase(socket);
                }
                if ( this->socketsGate->count( socket ) != 0 ) {
                    this->socketsGate->erase(socket);
                }
                this->fileDescriptors->remove( socket );
//                cerr << "   "  << "error ready " << socket << endl;
    //            info = this->sockets->find( socket )->second;
    //            info.second->onMessageError( *info.first );
            }
        }
    } while( !_stop );

}

void Listener::waitUntilReady() {
    timeval tv = {1,0};
    this->fileDescriptors->sort();

    memcpy(this->read_fd_set,this->master_fd_set,sizeof( *this->master_fd_set) );
//    memcpy(this->write_fd_set,this->master_fd_set,sizeof( *this->master_fd_set) );
    memcpy(this->error_fd_set,this->master_fd_set,sizeof( *this->master_fd_set) );

//    cerr << "max sock " << this->fileDescriptors->back() << endl;
    if ( select( this->fileDescriptors->back() +1, this->read_fd_set, NULL/*this->write_fd_set*/, this->error_fd_set, NULL) == -1 ) {
        error << " el select salio mal " << endl;
    };
}


template void Listener::add(PointClass<line>*, SocketEventClass*);

