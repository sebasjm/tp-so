#include "configuration.h"

#include <iostream>
#include <fstream>
#include <string>

#include <libxml/parser.h>
#include <libxml/xmlstring.h>
#include <map>
#include <iostream>

#include "../model/target.h"
#include "../model/result.h"
#include "../model/taskResult.h"
#include "../model/task.h"
#include "../model/job.h"
#include "../model/jobSet.h"

using namespace std;

#define JOB 1
#define TASK_RESULT 2

struct tiposDeNodos {
    int tipo_estructura;
    string nombre_nodo;
    void* (*reader)(xmlNode*) ;
};

template<typename type>
map<string,list<void*>* >* readConfig(type nodo);

void* readTaskResultNode(xmlNode* nodo);
void* readResultNode(xmlNode* nodo);

void* readJobNode(xmlNode* nodo);
void* readDependencyNode(xmlNode* nodo);
void* readResourceNode(xmlNode* nodo);
void* readTargetNode(xmlNode* nodo);
void* readTaskNode(xmlNode* nodo);

void* readStringNode(xmlNode* nodo);
void* readListNode(xmlNode* nodo);

void* get_from_map(char* name, map<string, list<void*>*>* values);
string* get_string_from_map(char* name, map<string, list<void*>*>* values);
list<void*>* get_list_from_map(char* name, map<string, list<void*>*>* values);

static tiposDeNodos nodos[] = {
    {JOB,"job",readJobNode},
    {JOB,"dependency",readDependencyNode},
    {JOB,"resource",readResourceNode},
    {JOB,"target",readTargetNode},
    {JOB,"task", readTaskNode},

    {JOB,"hash", readStringNode},
    {JOB,"id", readStringNode},
    {JOB,"method", readStringNode},
    {JOB,"length", readStringNode},
    {JOB,"task-id", readStringNode},
    {JOB,"task-description", readStringNode},
    {JOB,"name", readStringNode},
    {JOB,"value", readStringNode},
    {JOB,"ref", readStringNode},

    {JOB,"tasks", readListNode},
    {JOB,"resources", readListNode},
    {JOB,"dependencies", readListNode},

    {TASK_RESULT,"taskResult", readTaskResultNode},
    {TASK_RESULT,"result", readResultNode},
    {TASK_RESULT,"task", readStringNode},
    {TASK_RESULT,"parent", readStringNode},
    {TASK_RESULT,"status", readStringNode},
    {TASK_RESULT,"value", readStringNode},
    
    {0,"",NULL}
};

int tipo_de_archivo_de_configuracion = JOB;

JobClass* readJobClass(const char* file) {
    tipo_de_archivo_de_configuracion = JOB;
    xmlDocPtr doc = xmlReadFile(file,NULL,XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NONET  );
    if (!doc) return NULL;
    xmlNodePtr cur = xmlDocGetRootElement(doc);
    if (!cur) return NULL;

    return (JobClass*) readJobNode( cur );
}

TaskResultClass* readTaskResultClass(const char* file) {
    tipo_de_archivo_de_configuracion = TASK_RESULT;
    xmlDocPtr doc = xmlReadFile(file,NULL,XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NONET  );
    if (!doc) return NULL;
    xmlNodePtr cur = xmlDocGetRootElement(doc);
    if (!cur) return NULL;

    return (TaskResultClass*) readTaskResultNode( cur );
}

void* readTaskResultNode(xmlNode* nodo){
    map<string, list<void*>* >* values = readConfig<xmlAttrPtr>( nodo->properties );
    string* task = (string*) get_string_from_map("task",values);
    string* parent = (string*) get_string_from_map("parent",values);
    values = readConfig( nodo->children );
    ResultClass* result = (ResultClass*) get_from_map("result",values);
    return new TaskResultClass(task,parent,result);
}

void* readResultNode(xmlNode* nodo){
    map<string, list<void*>* >* values = readConfig( nodo->children );
    string* status = (string*) get_string_from_map("status",values);
    string* value = (string*) get_string_from_map("value",values);
    return new ResultClass(status,value);
}

void* readJobNode(xmlNode* nodo){
    map<string, list<void*>* >* values = readConfig<xmlAttrPtr>( nodo->properties );
    string* id = (string*) get_string_from_map("id",values);
    values = readConfig( nodo->children );
    TargetClass* target = (TargetClass*) get_from_map("target",values);
    list<TaskClass*>* tasks = (list<TaskClass*>*) get_list_from_map("tasks",values);
    return new JobClass(id, target, tasks);
}

void* readDependencyNode(xmlNode* nodo){
    map<string, list<void*>* >* values = readConfig<xmlAttrPtr>( nodo->properties );
    string* ref = (string*) get_string_from_map("ref",values);
    return new DependencyClass(*ref);
}

void* readResourceNode(xmlNode* nodo){
    map<string, list<void*>* >* values = readConfig<xmlAttrPtr>( nodo->properties );
    string* name = (string*) get_string_from_map("name",values);
    string* value = (string*) get_string_from_map("value",values);
    return new ResourceClass(*name,*value);
}

void* readTargetNode(xmlNode* nodo){
    map<string, list<void*>* >* values = readConfig( nodo->children );
    string* hash = (string*)get_string_from_map("hash",values);
    string* strLength = (string*)get_string_from_map("length",values);
//    string* method = (string*) values.find("method")->second;
    return new TargetClass(*hash, atoi(strLength->data()) );
}

void* readTaskNode(xmlNode* nodo){
    map<string, list<void*>* >* values = readConfig( nodo->children );
    string* taskId = (string*) get_string_from_map("task-id",values);
    string* taskDescription = (string*) get_string_from_map("task-description",values);

    list<ResourceClass*>* resources = (list<ResourceClass*>*) get_list_from_map("resources",values);
    list<DependencyClass*>* dependencies = (list<DependencyClass*>*) get_list_from_map("dependencies",values);

    return new TaskClass(taskId,taskDescription,resources,dependencies);
}

/**
 * Completa un nodo de tipo texto
 */
void* readStringNode(xmlNode* nodo){
    return new string((const char*)nodo->children->content);
}

/**
 * Completa un nodo de tipo lista
 */
void* readListNode(xmlNode* nodo){
    if ( !nodo->children ) return NULL;
    map<string, list<void*>* >* values = readConfig( nodo->children );
    if ( !nodo->children->next ) return new list<void*>();
    return values->find( (const char*)nodo->children->next->name )->second;
}

/**
 * Parsea el archivo de xml.
 * Cuando encuentra un nodo invoca a la funcion correspondiente para
 * interpretarla.
 * Mantiene un mapa<nombreNodo,listaDeInstancias> donde la listaDeInstancias
 * son las instancias representativas de cada nodo con sus valores
 *
 */
template<typename type>
map<string,list<void*>* >* readConfig(type nodo) {
    map<string,list<void*>* >* result = new map<string,list<void*>* >();
    int i = 0;
    while (nodo != NULL) {

        while ( nodos[i].reader != NULL && nodo != NULL) {
            if (nodos[i].nombre_nodo.compare( (const char*)nodo->name ) == 0 && tipo_de_archivo_de_configuracion == nodos[i].tipo_estructura ) {
                void* item = (void*)nodos[i].reader((xmlNodePtr)nodo);

                if ( result->count( nodos[i].nombre_nodo ) == 0 ) {
                    result->insert(
                        pair<string,list<void*>* >(
                            nodos[i].nombre_nodo,
                             new list<void*>()
                        )
                    );
                }
                result->find( nodos[i].nombre_nodo )->second->push_front(item);

                break;
            }
            i++;
        }

        nodo = nodo->next;
        i = 0;
    }
    return result;
}

void* get_from_map(char* name, map<string, list<void*>*>* values) {
    if ( values->find(name) == values->end() ) return NULL;
    return values->find(name)->second->front();
}

string* get_string_from_map(char* name, map<string, list<void*>*>* values) {
    void* result = get_from_map(name,values);
    if (result == NULL) return new string("");
    return (string*)result;
}
list<void*>* get_list_from_map(char* name, map<string, list<void*>*>* values) {
    void* result = get_from_map(name,values);
    if (result == NULL) return new list<void*>();
    return (list<void*>*)result;
}
