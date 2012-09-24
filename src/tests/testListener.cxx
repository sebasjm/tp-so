#include <iostream>

#include <ipc/socket.h>
#include <messages/interface-daemon.h>
#include <messages/teclado-consola.h>

#include <ipc/socketBuilder.h>
#include <ipc/tecladoConsola.h>

#include <ipc/listener.h>
#include <ipc/socketEvent.h>
#include <string.h>

#define PORT 3490 // the port client will be connecting to
#define SERVER_HOST "localhost"

#define MAXDATASIZE 10 // max number of bytes we can get at once
#define BACKLOG 10     // how many pending connections queue will hold

#include <util/log.h>
__init_log(testListener)
__init_logname(testListener)

class ConectorCliente: public SocketEventClass {

private:
    mensaje unMensaje;

public:
    ConectorCliente(mensaje unMensaje){
        this->unMensaje = unMensaje;
    }

    virtual bool onMessageRecv(PointClass<>* conector){
        SocketClass<line> sock ( *conector );
        line otroMensaje;

        sock >> otroMensaje;

        otroMensaje.str[sizeof(otroMensaje.str)] = '\0';
        if (sock.isClosed()) {
            debug << "se cerro la conexion" << endl;
        } else {
            cout
                << "[server:sock=" << sock.id() << "] "
                << otroMensaje.str
            << endl;
        }

        return sock.isClosed();
    };

    virtual pair<PointClass<>*, SocketEventClass*> onNewConnection(SocketGateClass* listener){
        SocketClass<>* sock = listener->acceptConnection();
        ( (SocketClass<mensaje>)*sock) << this->unMensaje;

        return pair<PointClass<>*, SocketEventClass*>( sock, this );
    };

    virtual void onMessageSent(PointClass<>* sock){};
    virtual void onMessageError(PointClass<>* sock){};
};

class ConectorTeclado: public SocketEventClass {

private:
    mensaje unMensaje;

public:
    ConectorTeclado(mensaje unMensaje){
        this->unMensaje = unMensaje;
    }

    virtual bool onMessageRecv(PointClass<>* conector){
        TecladoConsolaClass sock ( *conector );
        line str;
//        cerr << "reading buffer " << cin.rdstate() << " | " << cin.rdbuf() << endl;

        if ( cin.fail() ) {
//            cerr << " -------------- " << cin.rdstate() << endl;
            cin.clear();
            cin.ignore();
//            cerr << " -------------- " << cin.rdstate() << endl;
        }
        sock >> str;

//        otroMensaje[sizeof(otroMensaje)] = '\0';
        cout << "[keyboard] " << str.str << endl;
        
        return false;
    };

    virtual pair<PointClass<>*, SocketEventClass*> onNewConnection(SocketGateClass* listener){
        SocketClass<>* sock = listener->acceptConnection();
        ( (SocketClass<mensaje>)*sock) << this->unMensaje;
        
        return pair<PointClass<>*, SocketEventClass*>( sock, this );
    };

    virtual void onMessageSent(PointClass<>* sock){};
    virtual void onMessageError(PointClass<>* sock){};
};


int main(int argc,char** argv) {

    mensaje mensajeDeBienvenida;
    strncpy(mensajeDeBienvenida.str,"buenas !!!",sizeof(mensajeDeBienvenida.str) );

    SocketBuilderClass* builder = new SocketBuilderClass();
    builder->ip(SERVER_HOST);
    builder->puerto(PORT);

    info << "server: waiting for connections..." << endl;

    Listener* listener = new Listener();

    listener->add( builder->createBind() , new ConectorCliente( mensajeDeBienvenida ) );
    listener->add( new TecladoConsolaClass() , new ConectorTeclado( mensajeDeBienvenida ) );

    listener->loop();

    delete listener;
    return 0;
}