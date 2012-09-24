#include <stdio.h>

#include "shell.h"
#include <ipc/listener.h>
#include <ipc/socket.h>
#include <ipc/socketEvent.h>
#include <ipc/tecladoConsola.h>
#include <iostream>
#include <string.h>

#include "commands.h"

volatile commands_info nodo_commands[] = {
    { "start", &commands_start },
    { "stop", &commands_stop },
    { "display", &commands_display },
    { "refresh", &commands_refresh },
    { NULL, &commands_help }
};

volatile commands_info shell_commands[] = {
    { "exit", &commands_exit },
    { "quit", &commands_exit },
    { "list", &commands_list },
    { "running", &commands_running },
    { NULL, &commands_help }
};



class ConectorCliente: public SocketEventClass {

private:

public:
    virtual bool onMessageRecv(PointClass<>* conector){
        TecladoConsolaClass sock ( *conector );
        
        line otroMensaje;
        sock >> otroMensaje;

        char* nombre_nodo = strtok(otroMensaje.str, " ");
        char* comando = strtok(NULL, " ");

        command_func func = NULL;
        for(int i=0; func == NULL && shell_commands[i].name != NULL; i++) {
            if ( nombre_nodo != NULL && !strcmp(nombre_nodo,shell_commands[i].name) ) {
                func = shell_commands[i].resolver;
            }
        }
        for(int i=0; func == NULL && nodo_commands[i].name != NULL; i++) {
            if ( comando != NULL && !strcmp(comando,nodo_commands[i].name) ) {
                func = nodo_commands[i].resolver;
            }
        }
        if (func == NULL) func = &commands_help;
        
        func(sock,nombre_nodo);

        return sock.isClosed();
    };

    virtual pair<PointClass<>*, SocketEventClass*> onNewConnection(SocketGateClass* listener){};
    virtual void onMessageSent(PointClass<>* sock){};
    virtual void onMessageError(PointClass<>* sock){};
};

void startShell() {
    Listener* listener = new Listener();

    listener->add( new TecladoConsolaClass() , new ConectorCliente( ) );

    listener->loop();
}
