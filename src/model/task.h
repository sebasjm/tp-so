#ifndef __task_h
#define __task_h

#include <string>
#include <list>

#include "resource.h"
#include "dependency.h"
#include "job.h"

using namespace std;

class TaskClass {
public:
    TaskClass(string* taskId, string* taskDescription, list<ResourceClass*>* resources, list<DependencyClass*>* dependencies);
    string* taskId();
    string* taskDescription();
    list<ResourceClass*>* resources();
    list<DependencyClass*>* dependencies();
    int time(class JobClass* job);
    bool tieneDeadlock(class JobClass* job);
private:
    bool buscarDeadlock(list<string*> task_ids, class JobClass* job);
    string* _taskId;
    string* _taskDescription;
    list<ResourceClass*>* _resources;
    list<DependencyClass*>* _dependencies;
};

#endif