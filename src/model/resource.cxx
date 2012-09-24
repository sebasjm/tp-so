#include "resource.h"

ResourceClass::ResourceClass(const string nombre, const string value):
    _nombre(nombre),
    _value(value){
}

const string ResourceClass::nombre() {
    return _nombre;
}

const string ResourceClass::value() {
    return _value;
}
