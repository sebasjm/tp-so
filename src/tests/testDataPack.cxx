#include <iostream>

#include <ipc/socket.h>
#include <messages/interface-daemon.h>

#include <ipc/socketBuilder.h>

#include <ipc/listener.h>
#include <ipc/socketEvent.h>
#include <string.h>

#define PORT 3490 // the port client will be connecting to

#include <util/log.h>
__init_log(testSocket)
__init_logname(testSocket)

int cliente(char*);
int server();

#include <messages/daemon-daemon.h>

int main(int argc,char** argv) {
    estructura_complicada header;
    cerr << " header int " << sizeof header.field1
            << " char " << sizeof header.field2
            << " long " << sizeof header.field3
            << " double " << sizeof header.field4
            << " float " << sizeof header.field5
            << " void* " << sizeof header.field6
            << " long long " << sizeof header.field7
            << " size " << sizeof header << endl;


    if (argc != 3) return 1;
    if (!strcmp("client",argv[1]))
        cliente(argv[2]);
    if (!strcmp("server",argv[1]))
        server();

    return 0;
}


int cliente(char* ip) {
    SocketBuilderClass* builder = new SocketBuilderClass();
    builder->ip(ip);
    builder->puerto(PORT);
    SocketClass<>* sock = builder->startConnection();
    SocketClass<estructura_complicada> sock_mensaje( sock->id() );

    estructura_complicada header = {
        1,
        'a',
        2,
        3.1415,
        4.12345,
        (void*)5,
        6l
    };
    
    sock_mensaje << header;

    return 0;
}

class ConectorCliente: public SocketEventClass {

public:

    virtual bool onMessageRecv(PointClass<>* conector){
        SocketClass<estructura_complicada> sock ( conector->id() );
        estructura_complicada header;

        sock >> header;

        if (sock.isClosed()) return true;
        
/*
 *     int field1;
    char field2;
    long field3;
    double field4;
    float field5;
    void* field6;
    long long field7;
*/
        cerr << " header int " << header.field1
                << " char " << header.field2
                << " long " << header.field3
                << " double " << header.field4
                << " float " << header.field5
                << " void* " << header.field6
                << " long long " << header.field7
                << endl;

        return false;
    };

    virtual pair<PointClass<>*, SocketEventClass*> onNewConnection(SocketGateClass* listener){
        return pair<PointClass<>*, SocketEventClass*>(
                listener->acceptConnection(),
                this
            );
    };

    virtual void onMessageSent(PointClass<>* sock){};
    virtual void onMessageError(PointClass<>* sock){};
};

int server() {

    SocketBuilderClass* builder = new SocketBuilderClass();
    builder->ip("0.0.0.0");
    builder->puerto(PORT);

    Listener* listener = new Listener();

    listener->add( builder->createBind() , new ConectorCliente( ) );
    listener->loop();

    return 0;
}
