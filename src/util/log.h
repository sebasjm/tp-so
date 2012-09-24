#ifndef __log_h
#define __log_h

#include <iostream>

using namespace std;

class LogClass {
public:
    typedef ostream& (*logLevel)(char*, char*);

    LogClass( logLevel , char*);
    static LogClass* instance(char* moduleName);

    ostream& operator<<(char* msg);

    static void error(char* msg);
    static void info(char* msg);
    static void debug(char* msg);

    void defaultLevel(logLevel);

private:
    logLevel _defaultLevel;
    char* moduleName;
};

char* get_log_name();

ostream& log_info(char* mod, char* msg);
ostream& log_debug(char* mod, char* msg);
ostream& log_error(char* mod, char* msg);

#define __init_log(s) \
namespace __log_class_##s { \
    LogClass debug = *(new LogClass( &log_debug , #s )); \
    LogClass error = *(new LogClass( &log_error , #s )); \
    LogClass info  = *(new LogClass( &log_info  , #s )); \
    \
} \
using namespace __log_class_##s;
//end define
#include <pthread.h>
#define __init_logname(s) \
    pthread_mutex_t mutexLog; \
    char* get_log_name() { \
        return "log/"#s;\
    }\
//end edfine

#endif
