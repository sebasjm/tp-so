#include "log.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <so_agnostic.h>

using namespace std;

extern pthread_mutex_t mutexLog;

LogClass::LogClass( logLevel defaultLevel , char* moduleName ) {
    this->_defaultLevel = defaultLevel;
    this->moduleName = moduleName;
}

ostream& LogClass::operator<<(char* msg) {
//    ofstream wwwasdasde("",ofstream::ate | ofstream::app);
    return this->_defaultLevel(this->moduleName, msg);
}

#include <time.h>

char* get_pid(char* buf) {
    sprintf(buf,"%7d",GET_PID);
    return buf;
}
char* get_time(char* buf) {
    time_t rawtime;
    time ( &rawtime );
    struct tm * timeinfo = NULL;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
	strftime(buf, 50, "%Y-%m-%d %H:%M:%S", timeinfo);
    return buf;
}

ofstream _log_file;

char* get_effective_log_name() {
    char pid_name[20];
    sprintf(pid_name,"-%d.log",GET_PID);
    char* log_name = get_log_name();
    int file_len = strlen(log_name) + strlen(pid_name);
    char* name = new char[file_len+1];
    strcpy(name,log_name);
    strcpy(&name[strlen(log_name)],pid_name);
    name[file_len] = '\0';
    return name;
}
#ifdef  __CHANCHADAS_MARCA_WINDOWS
#define __logging_thread_value
#else
#define __logging_thread_value << pthread_self()
#endif
ostream& log_info(char* mod, char* msg) {
	pthread_mutex_lock(&mutexLog);
    char str[50],pid[10];
    if (_log_file.is_open()) _log_file.close();
    char* log_name = get_effective_log_name();
    _log_file.open( log_name, ofstream::ate | ofstream::app);
    delete log_name;
    _log_file << get_time(str) << " {" << get_pid(pid) << " | " __logging_thread_value << "} [INFO ] (" << mod << ") : " << msg ;
    _log_file.flush();
    pthread_mutex_unlock(&mutexLog);
    return _log_file;
}

ostream& log_debug(char* mod, char* msg) {
	pthread_mutex_lock(&mutexLog);
    char str[50],pid[10];
    if (_log_file.is_open()) _log_file.close();
    char* log_name = get_effective_log_name();
    _log_file.open( log_name, ofstream::ate | ofstream::app);
    delete log_name;
    _log_file << get_time(str) << " {" << get_pid(pid) << " | "  __logging_thread_value << "} [DEBUG] (" << mod << ") : " << msg ;
    _log_file.flush();
    pthread_mutex_unlock(&mutexLog);
    return _log_file;
}


ostream& log_error(char* mod, char* msg) {
	pthread_mutex_lock(&mutexLog);
    char str[50],pid[10];
    if (_log_file.is_open()) _log_file.close();
    char* log_name = get_effective_log_name();
    _log_file.open( log_name, ofstream::ate | ofstream::app);
    delete log_name;
    _log_file << get_time(str) << " {" << get_pid(pid) << " | " __logging_thread_value << "} [ERROR] (" << mod << ") - \""<< "\" : " << msg ;
    _log_file.flush();
    pthread_mutex_unlock(&mutexLog);
    return _log_file;
}

void LogClass::defaultLevel( logLevel level ) {
    this->_defaultLevel = level;
}

#include <unistd.h>
#include <stdarg.h>

void ansi_log( LogClass::logLevel log_writer, char* mod, char* str, va_list args) {
    char* s;
    vasprintf(&s,str,args);
    log_writer(mod, s);
    free(s);
}

void info(char* mod, char* str, ...) {
    va_list args;
    va_start(args, str);
    ansi_log( &log_info, mod, str, args);
    va_end(args);
}

void debug(char* mod, char* str, ...) {
    va_list args;
    va_start(args, str);
    ansi_log( &log_debug, mod, str, args);
    va_end(args);
}

void error(char* mod, char* str, ...) {
    va_list args;
    va_start(args, str);
    ansi_log( &log_error, mod, str, args);
    va_end(args);
}



