#include "commands.h"
#include "messages/daemon-daemon.h"
#include "node/daemon/networkManager.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <util/config.h>

#include <ipc/socketBuilder.h>
#include <ipc/socket.h>
#include <node/daemon/handshake.h>
#include <messages/daemon-daemon.h>

#include <util/log.h>

using namespace std;

__init_log(commands)

char* build_config_name(char* nodo_name) {
    char* nodo_config_dir = read_config("./config/interfaz.xml","nodo_config_dir");
    int length_configName = strlen(nodo_name) + strlen( nodo_config_dir ) + strlen(".xml") +1;
    char* result = (char*) malloc(length_configName);
    result[0] = '\0';
    strcat(result, nodo_config_dir );
    strcat(result,nodo_name);
    strcat(result,".xml");
    result[length_configName] = '\0';
//    free(nodo_config_dir);
    return result;
}

void commands_start   (TecladoConsolaClass teclado, char* nombre_nodo) {
//    debug << " comando start en el nodo " << nombre_nodo << endl;
    
    if (!fork()) {
        char* argv[3];
        argv[0] = read_config("./config/interfaz.xml","daemon.bin");
        argv[1] = build_config_name(nombre_nodo);;
        argv[2] = NULL;

        setsid();
        execv(argv[0],argv);
        error << " start | nunca deberia llegar aca" << endl;
    }
}

void commands_stop    (TecladoConsolaClass teclado, char* nombre_nodo) {
//    debug << " comando stop en el nodo " << nombre_nodo << endl;

    char* configName = build_config_name(nombre_nodo);

    SocketBuilderClass* builder = new SocketBuilderClass();
    builder->ip( read_config(configName,"mi_ip") );
    builder->puerto( read_config(configName,"mi_puerto") );
    SocketClass<>* sock = (SocketClass<>*) builder->startConnection();
    if (sock != NULL && client_handshake(sock) ) {
        SocketClass<insomnio_header> sock_header( sock->id() );
        insomnio_header header;
        header.payload = TYPE_STOP;
        header.length = 0;
        sock_header << header;

        sock_header.stop();
    }
    if (sock!=NULL) delete sock;
    delete builder;
}

void commands_refresh (TecladoConsolaClass teclado, char* nombre_nodo) {
//    debug << " comando refresh en el nodo " << nombre_nodo << endl;

    char* configName = build_config_name(nombre_nodo);

    SocketBuilderClass* builder = new SocketBuilderClass();
    builder->ip( read_config(configName,"mi_ip") );
    builder->puerto( read_config(configName,"mi_puerto") );
    SocketClass<>* sock = (SocketClass<>*) builder->startConnection();
    if (sock != NULL && client_handshake(sock) ) {
        SocketClass<insomnio_header> sock_header( sock->id() );
        insomnio_header header;
        header.payload = TYPE_REFRESH;
        header.length = 0;
        sock_header << header;

        sock_header.stop();
    }
    if (sock!=NULL) delete sock;
    delete builder;
}

void commands_display (TecladoConsolaClass teclado, char* nodoName) {
    char* configName = build_config_name(nodoName);
    
    SocketBuilderClass* builder = new SocketBuilderClass();
    builder->ip( read_config(configName,"mi_ip") );
    builder->puerto( read_config(configName,"mi_puerto") );
    SocketClass<>* sock = (SocketClass<>*) builder->startConnection();
    if (sock != NULL && client_handshake(sock) ) {
        SocketClass<insomnio_header> sock_header( sock->id() );
        SocketClass<info_nodo> sock_info( sock->id() );
        SocketClass<nombre_nodo> sock_nombre( sock->id() );
        
        insomnio_header header;
        header.payload = TYPE_DISPLAY;
        header.length = 0;
        sock_header << header;

        info_nodo listo_el_pollo = {"","","","",0,0,0,0,0};
        info_nodo buf = listo_el_pollo;
        nombre_nodo pelada_la_gallina = {""};

        sock_info >> buf;
        cerr << "nodo: " << buf.nombre << " saliente: " << buf.nombre_saliente << endl;
        fprintf(stderr,"disco: %lu mem: %lu dc: %s \n",buf.disco,buf.memoria,(buf.dc == 0 ? "no" : "si" ));
//        cerr << "disco: " << buf.disco << " mem: " << buf.memoria << " dc: " << (buf.dc == 0 ? "no" : "si" ) << endl;
        cerr << "ip: " << buf.ip << " puerto: " << buf.puerto << endl;
        
        int es_dc = buf.dc;

        cerr << " -- " << nodoName << " -- conexiones entrantes : ";

        sock_info >> buf;
        while (strcmp(buf.nombre, "listo")) {
            cerr << buf.nombre << " (" << buf.distancia << ", "<< buf.dc <<") - ";
            sock_info >> buf;
        }
        cerr << endl;
        
        buf = listo_el_pollo;
        cerr << " -- " << nodoName << " -- conexiones salientes : ";

        sock_info >> buf;
        while (strcmp(buf.nombre, "listo")) {
            cerr << buf.nombre << " (" << buf.distancia << ", "<< buf.dc <<") - ";
            sock_info >> buf;
        }
        cerr << endl;

        cerr << " -- " << nodoName << " -- red : " ;

        while (strcmp(pelada_la_gallina.id, "listo")) {
            sock_nombre >> pelada_la_gallina;
            cerr << pelada_la_gallina.id << " -- ";
        }
        cerr << endl;

        if ( es_dc ) {
            debug << " me dijo que es DC asi que voy a esperar la info q falta " << endl;
            SocketClass<DCState> sock_state( sock->id() );
            DCState state;
            sock_state >> state;
            cerr << " -- " << nodoName << " -- dc state : job->" << state.job_id << endl;
            cerr << "\t\tjobs " << state.cant_jobs << "\ttasks " << state.cant_task << endl;
            cerr << "\t\twaiting " << state.cant_wait_task << "\trunning " << state.cant_running_task << endl;
        }

        sock_header.stop();
    }
    if (sock!=NULL) delete sock;
    delete builder;
//    cout << " comando display en el nodo " << nombre_nodo << endl;
}

void commands_help    (TecladoConsolaClass sock, char* nombre_nodo) {
    char* ayuda = read_config("./config/interfaz.xml","ayuda");
    char* tmp = ayuda;
    while ( *(++tmp) ) if (*tmp == '|') *tmp = '\n';
    cout << ayuda << endl;
}

void commands_exit    (TecladoConsolaClass sock, char* nombre_nodo) {
    exit(0);
}

void commands_list  (TecladoConsolaClass sock, char* nombre_nodo) {
    char* nodo_config_dir = read_config("./config/interfaz.xml","nodo_config_dir");
    if (!fork()) {
        char* argv[3];
        argv[0] = "/bin/ls";
        argv[1] = nodo_config_dir;
        argv[2] = NULL;

        setsid();
        execv(argv[0],argv);
        error << " list | nunca deberia llegar aca" << endl;
    }
}

void commands_running (TecladoConsolaClass sock, char* nombre_nodo) {
    system("ps -fea | grep build");
}
