#include "configuration.h"
#include "model/taskResult.h"

#include <messages/dc-messages.h>
#include <xml/configuration.h>
#include <string.h>
#include <stdlib.h>
#include <model/resource.h>
#include <model/target.h>
#include <model/job.h>
#include <iostream>

using namespace std;

#include <util/log.h>
__init_log(ansi_configuration)


TaskResult* readTaskResultStruct(const char* file) {
    TaskResultClass* clsTaskResult = readTaskResultClass(file);
    if (clsTaskResult == NULL) return NULL;
    TaskResult* stTaskResult = new TaskResult;
    strcpy(stTaskResult->id_job, clsTaskResult->parent()->data());
    strcpy(stTaskResult->id_task, clsTaskResult->task()->data());
    
    stTaskResult->status = TASK_UNKNOWN;
    if (clsTaskResult->result()->status()->data(),"TASK_SUCCESS")
        stTaskResult->status = TASK_SUCCESS;
    if (clsTaskResult->result()->status()->data(),"TASK_COMPLETE")
        stTaskResult->status = TASK_COMPLETE;
    if (clsTaskResult->result()->status()->data(),"TASK_FAIL")
        stTaskResult->status = TASK_FAIL;
    
    strcpy(stTaskResult->value, clsTaskResult->result()->value()->data());
    return stTaskResult;
}

#include <util/weighted.h>

Job* readJobStruct(const char* file) {
    JobClass* clsJob = readJobClass(file);
    if (clsJob == NULL) return NULL;
    Job* stJob = new Job;
    strcpy(stJob->id, clsJob->id()->data());
    strcpy(stJob->target.hash, clsJob->target()->hash().data());
    stJob->target.length = clsJob->target()->length();
    stJob->target.method = MD5;

    //completar info de las tareas, cargar todo menos las dependencias
    stJob->tasks = NULL;
    for (list<TaskClass*>::iterator it = clsJob->tasks()->begin(); it != clsJob->tasks()->end(); it++) {
        Tasks* head = new Tasks;
        head->next = NULL;
        head->task = new Task;
        head->task->dependencies = NULL;
        strcpy(head->task->id, (*it)->taskId()->data());
        strcpy(head->task->description, (*it)->taskDescription()->data());
        head->task->status.status = TASK_DEFAULT_STATUS;

        for (list<ResourceClass*>::iterator res = (*it)->resources()->begin(); res != (*it)->resources()->end(); res++) {
            if (!strcmp("cpu",(*res)->nombre().data()) )
                head->task->resources.cpu = atoi((*res)->value().data());
            if (!strcmp("disk",(*res)->nombre().data()) ) {
                char* disk = (char*)(*res)->value().data();
                head->task->resources.disk = getMultiplierUnit(&disk[strlen(disk)-2]) * atoi(disk);
            }
            if (!strcmp("memory",(*res)->nombre().data()) ) {
                char* mem = (char*)(*res)->value().data();
                head->task->resources.memory = getMultiplierUnit(&mem[strlen(mem)-2]) * atoi(mem);
            }
        }

        if (stJob->tasks == NULL) {
            stJob->tasks = head;
        } else {
            head->next = stJob->tasks;
            stJob->tasks = head;
        }


    }

    //cargar dependencias
    Tasks* stTask = NULL;
    for (list<TaskClass*>::iterator clsTask = clsJob->tasks()->begin(); clsTask != clsJob->tasks()->end(); clsTask++) {

        //buscar la tarea en la estructura
        stTask = stJob->tasks;
        while (stTask != NULL && strcmp(stTask->task->id, (*clsTask)->taskId()->data()) ) {
            stTask = stTask->next;
        }
        
        if (stTask == NULL) {
            error << "[NO DEBERIA PASAR] no se encontro la tarea que se acaba de agregar!! " << "taskId" << (*clsTask)->taskId()->data() << endl;
            return NULL;
        }

        Task* task = stTask->task;
        task->dependencies = NULL;

        for (list<DependencyClass*>::iterator dependency = (*clsTask)->dependencies()->begin(); dependency != (*clsTask)->dependencies()->end(); dependency++) {

            //buscar la dependencia en la lista de tareas de la estructura
            stTask = stJob->tasks;
            while (stTask != NULL && strcmp(stTask->task->id, (*dependency)->nombre().data() )) {
                stTask = stTask->next;
            }

            if (stTask == NULL) {
                error << "[NO DEBERIA PASAR] hay una dependencia que no se cumple" << "taskId" << (*dependency)->nombre().data() << endl;
                return NULL;
            }
            Task* dependencyTask = stTask->task;

            //teniendo la tarea en la estructura, agregar la dependencia
            Tasks* head = new Tasks;
            head->task = dependencyTask;
            head->next = NULL;
            
            if (task->dependencies == NULL) {
                task->dependencies = head;
            } else {
                head->next = task->dependencies;
                task->dependencies = head;
            }
            
        }
    }

    return stJob;
}
