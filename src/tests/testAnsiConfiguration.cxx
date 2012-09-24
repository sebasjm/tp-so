#include <xml/configuration.h>
#include <stdio.h>
#include <xml/ansi/configuration.h>

#define ESPACIO "   "

#include <util/log_ansi.h>
#define modulo "testAnsiConfiguration"
__init_logname("testAnsiConfig")

void printTasks(Tasks* tasks, int deepness){
    Tasks* i = tasks;
    while( i != NULL) {
        debug(modulo,"%s","\n");
        for (int j = 0; j < deepness; j++)
            debug(modulo,"%s",ESPACIO);
        debug(modulo,"%s","Task(taskId=");
        debug(modulo,"%s",i->task->id);
        debug(modulo,"%s",",taskDescription=\"");
        debug(modulo,"%s",i->task->description);
        debug(modulo,"%s","\"");
        debug(modulo,"%s",",resources=[");
        debug(modulo,"%s","Resources(cpu=\"");
        debug(modulo,"%d",i->task->resources.cpu);
        debug(modulo,"%s",",disk=\"");
        debug(modulo,"%d",i->task->resources.disk);
        debug(modulo,"%s",",mem=\"");
        debug(modulo,"%d",i->task->resources.memory);
        debug(modulo,"%s","],dependencies=[");
        printTasks( i->task->dependencies , deepness + 2);
        debug(modulo, "%s", "],");
        i = i->next;
    }
}

int main(int argc,char** argv) {

    Job* job = readJobStruct(argv[1]);
    if (job == NULL) {
        debug(modulo,"No se pudo imprimir el job \n");
        return 0;
    }

    debug(modulo,"%s","Job(target=");
    debug(modulo,"%s","Target(hash=");
    debug(modulo,"%s",job->target.hash);
    debug(modulo,"%s",",length=");
    debug(modulo,"%d",job->target.length);
    debug(modulo,"%s",",method=");
    debug(modulo,"%d",job->target.method);
    debug(modulo,"%s","),");
    debug(modulo,"%s","\n  tasks=[");
    printTasks(job->tasks, 2);
    debug(modulo,"%s","\n  ]");
    debug(modulo,"%s",")\n");
//    ofstream file("asd",ios_base::out);
//    file << argv[2] << endl;
//    file << str.str() << endl;
//    file << ( (str.str().compare(argv[2]) == 0)? "ok" : "fail") << endl;
//    cout << ( (str.str().compare(argv[2]) == 0)? "ok" : "fail") << endl;


    return 0;
}
