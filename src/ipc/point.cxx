#include "point.h"

#include <messages/interface-daemon.h>
#include <messages/teclado-consola.h>
#include <messages/webserver-buffer.h>
#include <messages/daemon-daemon.h>
#include <messages/dc-messages.h>

#include <fstream>
#include <string>

#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>

using namespace std;

template<typename type>
PointClass<type>::PointClass(int id) {
    this->_id = id;
}

template<typename type>
PointClass<type>::~PointClass() {
}

template<typename type>
const int PointClass<type>::id() {
    return this->_id;
}

template<typename type>
PointClass<type>::operator int() {
    return this->_id;
}

#define __define_point_templates( type ) \
template PointClass<type>::PointClass(int); \
template PointClass<type>::~PointClass(); \
template const int PointClass<type>::id(); \
template PointClass<type>::operator int(); 

__define_point_templates( void* )
__define_point_templates( mensaje )
__define_point_templates( char* )
__define_point_templates( line )
__define_point_templates( web_line )
__define_point_templates( info_nodo )
__define_point_templates( nombre_nodo )
__define_point_templates( msg_red )
__define_point_templates( insomnio_header )
__define_point_templates( Job )
__define_point_templates( Task )
__define_point_templates( DependencyNet )
__define_point_templates( NodeWorking )
__define_point_templates( AssignTask )
__define_point_templates( TaskResult )
__define_point_templates( DCState )
__define_point_templates( nodo_to_dc )
__define_point_templates( estructura_complicada )


