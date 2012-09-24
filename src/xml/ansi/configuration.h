#ifndef __ansi_configuration_h
#define __ansi_configuration_h

#ifdef __cplusplus
extern "C" {
#endif

#include <messages/dc-messages.h>

Job* readJobStruct(const char* file);
TaskResult* readTaskResultStruct(const char* file);


#ifdef __cplusplus
}
#endif

#endif
