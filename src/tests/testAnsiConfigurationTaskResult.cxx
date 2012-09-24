#include <xml/configuration.h>
#include <stdio.h>
#include <xml/ansi/configuration.h>

#define ESPACIO "   "

#include <util/log_ansi.h>
#define modulo "testAnsiConfigurationTaskResult"
__init_logname(modulo)

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
    TaskResult* task = readTaskResultStruct(argv[1]);
    if (task == NULL) {
        debug(modulo,"No se pudo imprimir el taskresult \n");
        return 1;
    }
    debug(modulo,"TaskResult(job=%s,status=%d,task=%s,value=%s)\n",task->id_job,task->status,task->id_task,task->value);

    return 0;
}
