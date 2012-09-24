#include "socket.h"

#include <messages/interface-daemon.h>
#include <messages/teclado-consola.h>
#include <messages/webserver-buffer.h>
#include <messages/daemon-daemon.h>
#include <messages/dc-messages.h>

#include <so_agnostic.h>
#include <string.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;
#include <util/tpl.h>
#include <util/log.h>
__init_log(socket)


template<typename type>
SocketClass<type>::SocketClass(const int id) : PointClass<type>(id) {
    this->_isClosed = false;
    this->_hasEnded = false;
}

template<typename type>
SocketClass<type>::~SocketClass() {

}

sockaddr_in get_info(int sock){
    sockaddr_in asd;
    #ifdef __CHANCHADAS_MARCA_WINDOWS
    int number = sizeof(sockaddr_in);
    #else
    socklen_t number = sizeof(sockaddr_in);
    #endif
    getsockname(sock, (struct sockaddr *)&asd ,&number);
    return asd;
}

#define ip_y_puerto(s) "[" << str_ip(s) << ":" << int_puerto(s) << "]"


template<typename type>
bool SocketClass<type>::isClosed() {
    return this->_isClosed;
}

template<typename type>
bool SocketClass<type>::hasEnded() {
    return this->_hasEnded;
}

#include <unistd.h>

template<typename type>
void SocketClass<type>::stop() {
    shutdown(this->_id,2);
    close(this->_id);
    this->_isClosed = true;
}

char* get_pack_info(void* s){return "";}
char* get_type_name(void* s){return "void*";}
template SocketClass<void*>::SocketClass(const int);
template SocketClass<void*>::~SocketClass();
template void SocketClass<void*>::stop();
template bool SocketClass<void*>::isClosed();

#define __define_socket_templates( desc, type ) \
char* get_pack_info(type s){return desc;} \
char* get_type_name(type s){return #type;} \
template SocketClass<type>::SocketClass(const int); \
template SocketClass<type>::~SocketClass(); \
template bool SocketClass<type>::isClosed(); \
template bool SocketClass<type>::hasEnded(); \
template void SocketClass<type>::stop(); \
template PointClass<type>& SocketClass<type>::operator <<(const type); \
template PointClass<type>& SocketClass<type>::operator >>(type&);\

//__define_socket_templates( "",void* )
__define_socket_templates( "",insomnio_header )
__define_socket_templates( "",msg_red )
__define_socket_templates( "",info_nodo )
__define_socket_templates( "",nombre_nodo )
__define_socket_templates( "",web_line )
__define_socket_templates( "",line )
__define_socket_templates( "",mensaje )
__define_socket_templates( "",Job )
__define_socket_templates( "",Task )
__define_socket_templates( "",DependencyNet )
__define_socket_templates( "",NodeWorking )
__define_socket_templates( "",AssignTask )
__define_socket_templates( "",TaskResult )
__define_socket_templates( "",DCState )
__define_socket_templates( "",nodo_to_dc )
__define_socket_templates( "S(iciffii)",estructura_complicada )


template<typename type>
PointClass<type>& SocketClass<type>::operator <<(const type value) {
//    void* data;
//    size_t size;

//    cerr << " writeing " << get_type_name(value) << endl;
//
//    tpl_node *tn;
//    tn = tpl_map(get_pack_info(value), &value);
//    tpl_pack(tn,0);
//    tpl_dump(tn, TPL_MEM, &data, &size);
//    cerr << " size " << size << endl;

    if ( send(this->_id,(const char*) &value, sizeof(type), 0) == -1 ){
        error << " no se pudo enviar mensaje tipo " << get_type_name(value) << " size " << sizeof(type) << " sock " << this->_id << " " << ip_y_puerto(this->_id) << endl;
    }

//    tpl_free(tn);
//    free(data);
    return *this;
}

template<typename type>
PointClass<type>& SocketClass<type>::operator >>(type& value) {
//    void* data;
//    size_t size;

//    cerr << " reading " << get_type_name(value) << endl;

//    tpl_node *tn;
//    tn = tpl_map(get_pack_info(value), &value);
//    tpl_dump(tn, TPL_GETSIZE, &size);
//    cerr << " size " << size << endl;
//    data = malloc(size);

    int count = recv(this->_id,(char*) &value, sizeof(type), 0);
    if ( count == -1) {
        error << " no se pudo recibir mensaje tipo " << get_type_name(value) << " size " << sizeof(type) << " sock " << this->_id << " " << ip_y_puerto(this->_id) << endl;
    }
    
//    tpl_load(tn, TPL_MEM, data, size);
//    tpl_unpack(tn,0);
//    tpl_free(tn);
//    free(data);
    this->_hasEnded = (count != sizeof(type) );
    this->_isClosed = (count == 0);

    if (this->_hasEnded) {
        char* ptr = (char*)&value;
        memset(&ptr[count],0,sizeof(type)-count);
    }

    return *this;
}

