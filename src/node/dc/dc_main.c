#include <util/log_ansi.h>
#include <node/dc/dc.h>
#include "ipc/ipcDaemon.h"
#include "messages/daemon-daemon.h"
#include <sys/shm.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#define modulo "dc_main"
__init_logname(modulo)

struct sigaction action;
sigset_t set;

int main(int argc, char** argv) {
        pthread_mutex_init(&mutexLog,NULL);
        FILE* f = fopen("dc.pid","w");
        fprintf(f,"%d",getpid());
        fclose(f);



	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	action.sa_handler = sigusrToMigration;
	sigaction(SIGUSR1, &action, NULL);

	debug(modulo, " init \n");

	initDC(argv);

	debug(modulo, " start \n");

	startDC(atoi(argv[3]));

	error(modulo, " no deberia llegar aca, significa q se esta cerrando\n");
}
