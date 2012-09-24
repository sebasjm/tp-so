#include <xml/configuration.h>

#include <iostream>
#include <fstream>
#include <sstream>

stringstream str( stringstream::in | stringstream::out );

using namespace std;

void printDependencies(list<DependencyClass*> deps){
    list<DependencyClass*>::const_iterator i = deps.begin();
    while( i != deps.end() ) {
        str << "Dependency(";
        str << "name=";
        str << (*i)->nombre();
        str << "),";
        i++;
    }

}

void printResources(list<ResourceClass*> res){
    list<ResourceClass*>::const_iterator i = res.begin();
    while( i != res.end() ) {
        str << "Resource(";
        str << "name=";
        str << (*i)->nombre();
        str << ",value=";
        str << (*i)->value();
        str << "),";
        i++;
    }

}

void printTasks(list<TaskClass*> tasks){
    list<TaskClass*>::const_iterator i = tasks.begin();
    while( i != tasks.end() ) {
        str << "Task(taskId=";
        str << *(*i)->taskId();
        str << ",taskDescription=\"";
        str << *(*i)->taskDescription();
        str << "\"";
        str << ",dependencies=[";
        printDependencies( *(*i)->dependencies() );
        str << "],resources=[";
        printResources( *(*i)->resources() );
        str << "]";
        i++;
    }
}

using namespace std;

int main(int argc,char** argv) {
    JobClass* job = readJobClass(argv[1]);
    str << "Job(target=";
    str << "Target(hash=";
    str << job->target()->hash();
    str << ",length=";
    str << job->target()->length();
    str << ",method=";
    str << job->target()->method();
    str << "),";
    str << "tasks=[";
    printTasks(*job->tasks());
    str << "]";
    str << ")";
    ofstream file("asd",ios_base::out);
    file << argv[2] << endl;
    file << str.str() << endl;
    file << ( (str.str().compare(argv[2]) == 0)? "ok" : "fail") << endl;
    cout << ( (str.str().compare(argv[2]) == 0)? "ok" : "fail") << endl;


    return 0;
}
