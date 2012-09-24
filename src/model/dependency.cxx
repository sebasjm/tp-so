#include "dependency.h"

#include <string>
using namespace std;

DependencyClass::DependencyClass(const string nombre):
    _nombre(nombre) {
}

const string DependencyClass::nombre(){
    return _nombre;
}