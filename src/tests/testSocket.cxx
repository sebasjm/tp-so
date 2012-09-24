#include <iostream>

#include <ipc/socket.h>
#include <messages/interface-daemon.h>

#include <ipc/socketBuilder.h>

#include <ipc/listener.h>
#include <ipc/socketEvent.h>
#include <string.h>

#define PORT 3490 // the port client will be connecting to
#define SERVER_HOST "127.0.0.1"

#define MAXDATASIZE 10 // max number of bytes we can get at once
#define BACKLOG 10     // how many pending connections queue will hold

#include <util/log.h>
__init_log(testSocket)
__init_logname(testSocket)

int cliente(char*);
int server();

int main(int argc,char** argv) {
    int a = fork();
    if (a) { // se divide un thread para el cliente y otro para el servidor
        server();
    } else {
//        for ( int i = 0; i < 3; i++ ) {
//            sleep(1);
//            cerr << "-----------------------------------" << endl;
            sleep(1);
            cliente(argv[1]);
//            cerr << "-----------------------------------" << endl;
//        }
    }

    return 0;
}

class ConectorCliente: public SocketEventClass {

private:
    mensaje unMensaje;

public:
    ConectorCliente(mensaje unMensaje){
        this->unMensaje = unMensaje;
    }

    virtual bool onMessageRecv(PointClass<>* conector){
        SocketClass<mensaje> sock ( conector->id() );
        mensaje otroMensaje;
        
        sock >> otroMensaje;

        otroMensaje.str[sizeof(otroMensaje.str)] = '\0';
        if (sock.isClosed()) {
            cerr << "se cerro la conexion" << endl;
        } else {
            cerr 
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


/**
 * Cliente
 * Se conecta, espera un mensaje y lo muestra por pantalla
 * ============================================
 */


int cliente(char* saludo) {
    mensaje un_mensaje2;
    SocketBuilderClass* builder = new SocketBuilderClass();
    builder->ip(SERVER_HOST);
    builder->puerto(PORT);
    SocketClass<>* sock = builder->startUnconnected();
    SocketClass<mensaje> sock_mensaje( sock->id() );
    
    cerr << "client: connecting created" << endl;
    sleep(1);
    strcpy(un_mensaje2.str,saludo);
    sock_mensaje << un_mensaje2;

    cerr << "client: received '" << un_mensaje2.str << "'" << endl;

//    strncpy(un_mensaje2.str,"callate",20);
//
//    sock_mensaje << un_mensaje2;
//
//    cerr << "client: enviado '" << un_mensaje2.str << "'" << endl;

//    close(sock_mensaje.id());
    return 0;
}

/**
 * Servidor
 * Registra un puerto para escuchar, espera que se conecte un cliente
 * y le envia un mensaje que recibe por parametro
 * ============================================
 */

int server() {

    mensaje un_mensaje;
    SocketBuilderClass* builder = new SocketBuilderClass();
    builder->ip(SERVER_HOST);
    builder->puerto(PORT);

    cerr << "server: waiting for connections..." << endl;

    Listener* listener = new Listener();

    listener->add( builder->createUnbinded() , new ConectorCliente( un_mensaje ) );
//    listener->add( new SocketClass<>(0) , new ConectorCliente( un_mensaje ) );
    cerr << " lupeando " << endl;
    listener->loop();
    
//    cerr << "1server: sending massage...." << endl;
//    SocketGateClass* gate = builder->createBind();
//    cerr << "2server: sending massage...." << endl;
//    SocketClass<>* sock = gate->acceptConnection();
//    cerr << "3server: sending massage...." << endl;
//    sleep(3);
//    cerr << "4server: sending massage...." << endl;
//    // le mandamos un mensaje
//    ((SocketClass<mensaje>)*sock) << un_mensaje;
//
//    // y esperamos recibir otro
//    ((SocketClass<mensaje>)*sock) >> un_mensaje;
//
//    cerr << un_mensaje.str << endl;
//    delete listener;
    return 0;
}
