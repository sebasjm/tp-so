#ifndef __job_h
#define __job_h

#include "target.h"
#include "task.h"

#include <list>
#include <string>

using namespace std;

class JobClass {
public:
    JobClass(string* id, TargetClass* target, list<class TaskClass*>* tasks);
    string* id();
    string* xml();
	void setXml(string* xmlcode);
    TargetClass* target();
    list<class TaskClass*>* tasks();
    int time();
    bool tieneDeadlock();
private:
    string* _id;
	string* _xml;
    TargetClass* _target;
    list<class TaskClass*>* _tasks;
};

#endif
