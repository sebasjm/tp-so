#include "result.h"

ResultClass::ResultClass(string* status, string* value) {
    _value = value;
    _status = status;
}

string* ResultClass::status() {
    return _status;
}

string* ResultClass::value() {
    return _value;
}
