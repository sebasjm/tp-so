#include "taskResult.h"

#include <string>

TaskResultClass::TaskResultClass(string* task, string* parent, ResultClass* result){
    _task = task;
    _parent = parent;
    _result = result;
}

string* TaskResultClass::task() {
    return _task;
}

string* TaskResultClass::parent() {
    return _parent;
}

ResultClass* TaskResultClass::result(){
    return _result;
}
