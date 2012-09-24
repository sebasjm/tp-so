#ifndef __log_ansi_h
#define __log_ansi_h

void info (char* mod, char * str, ...);
void debug(char* mod, char * str, ...);
void error(char* mod, char * str, ...);

char* get_log_name();

#include <pthread.h>
#define __init_logname(s) \
    pthread_mutex_t mutexLog; \
    char* get_log_name() { \
        return "log/"s;\
    }

#endif
