#include <iostream>
#include <ipc/point.h>
#include <ipc/socket.h>
#include <ipc/socketBuilder.h>
#include <ipc/listener.h>
#include <limits.h>

#include <webserver/conectorCliente.h>
#include <xml/configuration.h>
#include <util/config.h>

#include <util/log.h>
__init_log(webserver)
__init_logname(webserver)

#include "db.h"

void web_server(list<JobClass*>* ready_queue) {
    SocketBuilderClass* builder = new SocketBuilderClass();
    builder->ip( read_config("config/webserver.xml","listen_ip") );
    builder->puerto( read_config("config/webserver.xml","listen_port") );

    Listener* listener = new Listener();
    listener->add( builder->createBind() , new ConectorCliente( ready_queue ) );
    listener->loop();
}

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

char* dir_absoluto(char* file) {
    char* directory = read_config("config/webserver.xml","directory");
    int len = strlen(file);
    int lendir = strlen( directory );
    char* result = new char[len+lendir+1];
    result[0] = '\0';
    strcat(result, directory);
    strcat(result,file);
    return result;
}

bool ya_esta_en_la_lista(JobClass* job, list<JobClass*>* ready_q){
    for (list<JobClass*>::iterator it = ready_q->begin(); it != ready_q->end(); it++) {
        if (!strcmp( job->id()->data(),(*it)->id()->data()) ) {
            return true;
        }
    }
    return false;
}

void agregarOrdenado(list<JobClass*>* ready_queue, JobClass* job) {
    list<JobClass*>::iterator it;
    for(it = ready_queue->begin(); it != ready_queue->end() && job->time() > (*it)->time() ; it++) {
        
    }
    ready_queue->insert(it, job);
}

void file_reader(list<JobClass*>* ready_queue) {
    DIR *dp;
    struct dirent *ep;

    while (true) {
        sleep(5);
        debug << " loop, buscando algo en los archivos " << endl;
        if ( (dp = opendir( read_config("config/webserver.xml","directory") ) ) == NULL ) break;
//        ready_queue->clear();
        while ( ep = readdir(dp) ) {
            if (!strcmp(ep->d_name,"..") || !strcmp(ep->d_name,".")|| !strcmp(ep->d_name,".svn") ) continue;
            string archivo( dir_absoluto(ep->d_name) );
            JobClass* job = readJobClass( archivo.data() );
            remove( archivo.data() );
            if ( job == NULL ) {
                debug << "El archivo " << archivo.data() << " era basura" << endl;
                continue;
            }
            if ( job->tieneDeadlock() ) {
                debug << "El job " << job->id()->data() << " tiene deadlock, omitiendo" << endl;
                db_error_set_value( (char*)job->id()->data(), "Deadlock entre tareas");
                continue;
            }
            if ( ya_esta_en_la_lista(job, ready_queue) ){
                debug << "El job " << job->id()->data() << " ya habia sido registrado" << endl;
                continue;
            }

            info << "Nuevo JOB " << job->id()->data() << endl;
            agregarOrdenado(ready_queue, job);
            for (list<JobClass*>::iterator it = ready_queue->begin(); it != ready_queue->end(); it++) {
                debug << "    job " << (*it)->id()->data() << " " << (*it)->time() << endl;
            }
        }
        (void) closedir(dp);
    }
}

#include <pthread.h>
#include <so_agnostic.h>

typedef void* (*func)(void*);

int main(int argc,char** argv) {
    pthread_mutex_init(&mutexLog,NULL);
    char * dir = read_config("config/webserver.xml","directory");
    info << "iniciando webserver {"  << dir << "}" << endl;

    #ifdef __CHANCHADAS_MARCA_WINDOWS
    WSAData wsaData;
    WSAStartup(MAKEWORD(1,1),&wsaData);
    #endif
    
    #ifndef __CHANCHADAS_MARCA_WINDOWS
    FILE* f = fopen("ws.pid","w");
    fprintf(f,"%d",getpid());
    fclose(f);
    #endif


    list<JobClass*>* ready_queue = new list<JobClass*>();
    
    pthread_t thread_ws;
    pthread_t thread_fs;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + 1024);
	
    pthread_create(&thread_ws, NULL, (func)&web_server, (void *)ready_queue);
    pthread_create(&thread_fs, NULL, (func)&file_reader, (void *)ready_queue);

    pthread_attr_destroy(&attr);
   
    pthread_join(thread_ws,NULL);
    return 0;
}
