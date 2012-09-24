#include <iostream>
#include <string.h>
#include <ipc/socket.h>

#include "messages/teclado-consola.h"

#define CLIENT_INIT_MSG "INSOMNIO CONNECT/1.0\n\n"
#define SERVER_RESPONSE_MSG "INSOMNIO OK\n\n"

#include <util/log.h>
__init_log(handshake)


bool client_handshake(SocketClass<>* sock) {
    line inir_msg = {CLIENT_INIT_MSG};
    line response;
    SocketClass<line> init_hs( *sock );
    debug << "CHS " << endl;
    init_hs << inir_msg;
    debug << "CHS Wait" << endl;
    init_hs >> response;
    debug << "CHS Response" << endl;
    if ( !strcmp(response.str, SERVER_RESPONSE_MSG) ) {
        return true;
    }
    error << " handshake client " << str_ip( sock->id() ) << ":" << int_puerto(sock->id()) << endl;
    return false;
}

bool server_handshake(SocketClass<>* sock) {
    line response = {SERVER_RESPONSE_MSG};
    line client_msg;
    
    SocketClass<line> init_hs( *sock );
    debug << "SHS Wait " << endl;
    init_hs >> client_msg;
    debug << "SHS Recive" << endl;

    if ( strcmp(client_msg.str, CLIENT_INIT_MSG) ) {
        error << "handshake server " << str_ip( sock->id() ) << ":" << int_puerto(sock->id()) << endl;
        return false;
    }
    
    init_hs << response;
    debug << "SHS Send" << response.str << endl;

    return true;
}

