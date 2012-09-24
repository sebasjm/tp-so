#include <iostream>

#include <ipc/socket.h>
#include <ipc/listener.h>
#include <ipc/socketBuilder.h>

#include <messages/teclado-consola.h>
#include <util/config.h>
#include <string.h>
#include <stdlib.h>
#include <messages/daemon-daemon.h>
#include "handshake.h"

using namespace std;

#include "migration.h"
#include "netDiscovery.h"
#include "networkManager.h"

#include <util/log.h>
#include <node/dc/dc.h>
__init_log(daemon)
/*
        __init_logname(daemon)
 */

char log_name[40];

pthread_mutex_t mutexLog;
char* get_log_name() {
    return log_name;
}

int main(int argc,char** argv) {
    char* configName = argv[1];

    //Si no existe el archivo de configuracion, termina aca
    if (!fopen(configName,"r")) {
        cerr << " el archivo de configuracion " << (configName==NULL?"null":configName) << " no existe " << endl;
        return 1;
    }

    char* nombre = read_config(configName, "mi_nombre");
    pthread_mutex_init(&mutexLog,NULL);
    sprintf(log_name,"log/daemon_%s",nombre);
    debug << " init - config " << configName << endl;

    char pid_file[25];
    sprintf(pid_file,"%s.pid",nombre);
    FILE* f = fopen(pid_file,"w");
    fprintf(f,"%d",getpid());
    fclose(f);

    init_networkManager(configName);
}
