#include "job.h"
#include "resource.h"

JobClass::JobClass(string* id, TargetClass* target, list<TaskClass*>* tasks) {
    _target = target;
    _tasks = tasks;
    _id = id;
}

string* JobClass::id() {
    return _id;
}

int JobClass::time() {
    int max = 0;
    for(list<TaskClass*>::iterator it = _tasks->begin(); _tasks != NULL && it != _tasks->end(); it++) {
        int cpu = (*it)->time(this);
        if (cpu > max) max = cpu;
    }
    return max;
}

bool JobClass::tieneDeadlock() {
    for(list<TaskClass*>::iterator it = _tasks->begin(); _tasks != NULL && it != _tasks->end(); it++) {
        if ( (*it)->tieneDeadlock( this ) ) return true;
    }
    return false;
}

TargetClass* JobClass::target() {
    return _target;
}

list<TaskClass*>* JobClass::tasks() {
    return _tasks;
}

void JobClass::setXml(string* xmlcode) {
	_xml = xmlcode;
	return;
}

string* JobClass::xml() {
    return _xml;
}
