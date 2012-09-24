#include "tecladoConsola.h"
#include "socket.h"

#include <messages/interface-daemon.h>

#include <fstream>
#include <string>

#include <iostream>
#include <sys/socket.h>

using namespace std;

TecladoConsolaClass::TecladoConsolaClass(const int i) : PointClass<line>(i) {

}
TecladoConsolaClass::TecladoConsolaClass() : PointClass<line>(0) {

}

TecladoConsolaClass::~TecladoConsolaClass() {

}

PointClass<line>& TecladoConsolaClass::operator <<(const line value) {
    cout << value.str;
    return *this;
}

PointClass<line>& TecladoConsolaClass::operator >>(line& value){
    cin.getline(value.str, sizeof(line));
    return *this;
}

bool TecladoConsolaClass::isClosed() {
    return false;
}


