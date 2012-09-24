#include "target.h"

using namespace std;

TargetClass::TargetClass(string hash, int length):
    _hash(hash),
    _method("MD5"),
    _length(length) {
}

string TargetClass::hash(){
    return _hash;
}

int TargetClass::length() {
    return _length;
}

string TargetClass::method(){
    return _method;
}