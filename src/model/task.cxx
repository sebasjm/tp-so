#include "task.h"
#include "job.h"
#include "resource.h"
#include <string.h>
#include <stdlib.h>

TaskClass::TaskClass(string* taskId, string* taskDescription, list<ResourceClass*>* resources, list<DependencyClass*>* dependencies) {
    _taskId = taskId;
    _taskDescription = taskDescription;
    _resources = resources;
    _dependencies = dependencies;
}

string* TaskClass::taskId() {
    return _taskId;
}

TaskClass* getTaskById(const char* id, JobClass* job){
    for (list<TaskClass*>::iterator it = job->tasks()->begin(); it != job->tasks()->end(); it++) {
        if ( !strcmp(id, (*it)->taskId()->data() ) )
            return *it;
    }
    return NULL;
}

int getCpuTime(list<class ResourceClass*>* lista) {
    for (list<ResourceClass*>::iterator res = lista->begin(); res != lista->end(); res++) {
        if (!strcmp( (*res)->nombre().data(), "cpu")) {
            return atoi((*res)->value().data());
        }
    }
}

int TaskClass::time(JobClass* job) {
    int cpu = getCpuTime(_resources);
    int max, temp = 0;
    for (list<DependencyClass*>::iterator it = _dependencies->begin(); it != _dependencies->end(); it++) {
         temp = getTaskById( (*it)->nombre().data(), job)->time(job);
         if (temp > max) max = temp;
    }
    return cpu + temp;
}

bool TaskClass::tieneDeadlock( JobClass* job) {
    list<string*> ids;
    return this->buscarDeadlock( ids ,  job);
}

bool TaskClass::buscarDeadlock(list<string*> task_ids, JobClass* job) {
    for (list<string*>::iterator it = task_ids.begin(); it != task_ids.end(); it++) {
        if ( !strcmp(this->_taskId->data(), (*it)->data() ) ) return true;
    }
    task_ids.push_front( this->_taskId );
    for (list<DependencyClass*>::iterator it = _dependencies->begin(); it != _dependencies->end(); it++) {
         if ( getTaskById( (*it)->nombre().data(), job)->buscarDeadlock( task_ids, job ) ) return true;
    }
    return false;
}

string* TaskClass::taskDescription() {
    return _taskDescription;
}

list<ResourceClass*>* TaskClass::resources() {
    return _resources;
}

list<DependencyClass*>* TaskClass::dependencies() {
    return _dependencies;
}

