#include <model/job.h>
#include <model/task.h>
#include <model/target.h>
#include <model/dependency.h>
#include <model/resource.h>

#include <iostream>

using namespace std;

int main() {
    DependencyClass* dep = new DependencyClass("sebastian");
    cout << dep->nombre();
    list<DependencyClass*> deps;
    deps.push_front( dep );
    list<ResourceClass*> res;

    TaskClass* task = new TaskClass( new string("task-1"), new string("primera tarea"), &res, &deps );
    cout << "La lista tiene " << task->dependencies()->size() << " elementos";
    
    cout << "ok" << endl;
    return 0;
}
