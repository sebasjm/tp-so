#include "ipcstream.h"

#include <messages/interface-daemon.h>

#include <fstream>
#include <string>

#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>

using namespace std;

#include <util/log.h>
__init_log(ipcstream)



template<typename type>
IPCStreamClass<type>::IPCStreamClass(const string nombre) : PointClass<type>(0) {
    key_t key = ftok(nombre.data(),1);
    this->_id = msgget(key, 0666 | IPC_CREAT);
    this->_buf.mtype = 1;
}

template<typename type>
PointClass<type>& IPCStreamClass<type>::operator <<(type value){
    this->_buf.info = value;
    if ( msgsnd(this->_id, &this->_buf, sizeof(type), 0) ){
        error << " enviando msg size" << sizeof(type) << " sock " << this->_id << endl;
    }
    return *this;
}

template<typename type>
PointClass<type>& IPCStreamClass<type>::operator >>(type& value){
    if ( msgrcv(this->_id, &this->_buf, sizeof(type), 0, 0) == -1) {
        error << " reciviendo msg size" << sizeof(type) << " sock " << this->_id << endl;
    }
    value = this->_buf.info;
    return *this;
}

template<typename type>
PointClass<type>& IPCStreamClass<type>::read(type& value, int canal){
    if ( msgrcv(this->_id, &this->_buf, sizeof(type), canal, 0) == -1) {
        error << " reciviendo msg size" << sizeof(type) << " sock " << this->_id << endl;
    }
    value = this->_buf.info;
    return *this;
}

template IPCStreamClass<mensaje>::IPCStreamClass(const string);
template PointClass<mensaje>& IPCStreamClass<mensaje>::operator <<(mensaje);
template PointClass<mensaje>& IPCStreamClass<mensaje>::operator >>(mensaje&);
template PointClass<mensaje>& IPCStreamClass<mensaje>::read(mensaje&, int);


