#ifndef __configuration_h
#define __configuration_h

#include "../model/job.h"
#include "../model/taskResult.h"
using namespace std;

JobClass* readJobClass(const char* file);
TaskResultClass* readTaskResultClass(const char* file);

#endif
