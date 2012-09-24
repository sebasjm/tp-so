#include <iostream>
#include <ipc/point.h>
#include <ipc/socket.h>
#include <ipc/socketBuilder.h>
#include <ipc/listener.h>

#include <webserver/conectorCliente.h>
#include <xml/configuration.h>

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>

#include "shell.h"
#include <util/log.h>

typedef void* (*func)(void*);

__init_log(interface)
__init_logname(interface)

int main(int argc,char** argv) {
    pthread_t thread_iface;

    info << "init" << endl;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + 1024);

    pthread_create(&thread_iface, NULL, (func)&startShell, (void *)NULL);
    pthread_attr_destroy(&attr);
    pthread_join(thread_iface,NULL);
    return 0;
}
