#ifndef __taskResult_h
#define __taskResult_h

#include "result.h"

#include <string>

using namespace std;

class TaskResultClass {
public:
    TaskResultClass(string* task, string* parent, ResultClass* result);
    string* task();
    string* parent();
    ResultClass* result();
private:
    string* _task;
    string* _parent;
    ResultClass* _result;
};

#endif
