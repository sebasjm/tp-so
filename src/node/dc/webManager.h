
#ifndef WEBMANAGER_C_
#define WEBMANAGER_C_
#include <pthread.h>
#include <messages/dc-messages.h>

void initWM();
pthread_t startWM();
void stopWM();
void setJobWM(Job*);

void set_dc_ip_port(char* ip, char* puerto);

#endif /* WEBMANAGER_C_ */
