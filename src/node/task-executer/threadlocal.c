#include "threadlocal.h"
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <md5/md5.h>
#include "util/characters.c"
#include <time.h>
#include "taskExecuter.h"

typedef struct tempHash{
    time_t start;
	char* memHash;
	long  pointerMem;
	FILE* fileHash;
	long pointerFile;
	char* filename;
} TempHash ;

char *itoaLocal(long value);
void sendResult(ThreadData *data,TaskResponse *result){
	(*data->func)(result);
}

void closeThread(TempHash * tempHash){
	free(tempHash->memHash);
	free(tempHash->filename);
	fclose(tempHash->fileHash);
	remove(tempHash->filename);
	removeThread(pthread_self());
}

char* generateHash(char* value, int length){
	unsigned char res[SIZE_HASH];
	unsigned char resStr[SIZE_HASH+1];
	int i;
	memset ((void*)res, (int)'\0', (size_t)sizeof(res));
	md5_buffer (value, length, res);
	for (i=0; i<16; i++){
		sprintf ((char*)&resStr[i*2],"%02x",res[i]);
	}
	resStr[32] = '\0';
	return (char*)resStr;
}

int incrementValue(char* value, int length){
	if (length <= 0){
		return 0;
	}
	if (value[length-1] == characters[numberOfCharactares-1]){
		if (incrementValue(value, length-1)){
			value[length-1] = characters[0];
		}
	}else {
		value[length-1] = characters[positionOfCharacter(value[length-1]) + 1];
	}
	return 1;
}
/**
 * -1 = Complete, EJ busco de AAAAAA a BBBBBB y no lo encontro
 * 0  = Cuando no lo encuentra, y tiene que seguir buscando
 * 1  = Success en que dentro del rango se entro el valor correspondiente y lo encontre
 */

int searchHash(TaskResponse *result, TaskRun *task, TempHash *tempHash){
	if (strcmp(result->value, task->end) >= 0){
		return -1;
	}
	char* newHash = generateHash(result->value, task->length);
	if (strncmp(newHash,task->hash,SIZE_HASH) == 0)
		return 1;
	if ((task->memory - (tempHash->pointerMem + SIZE_HASH)) > 0 ){
		memcpy(tempHash->memHash + tempHash->pointerMem,newHash,SIZE_HASH);
		tempHash->pointerMem = tempHash->pointerMem + SIZE_HASH;
	} else {
		if ((task->disk - (tempHash->pointerFile + tempHash->pointerMem)) > 0 ){
			long cant = fwrite(tempHash->memHash, tempHash->pointerMem , sizeof(char), tempHash->fileHash);
			tempHash->pointerFile = tempHash->pointerFile + SIZE_HASH;
			tempHash->pointerMem = 0;
		}else{
			fclose(tempHash->fileHash);
			remove(tempHash->filename);
			tempHash->fileHash = fopen(tempHash->filename , "w"); /*Se puede usar tmpfile*/
			tempHash->pointerFile = 0;
		}
	}
	return 0;
}

#include <unistd.h>

void runThreadLocal (ThreadData *arg){
	Handle self = {arg->handle.thread};
	TaskResponse result = { self , 0 , NULL};
	TempHash *tempHash;
	float timeTranscured = 0;
	strcpy(result.value, arg->task.start);

	tempHash = (TempHash*)malloc(sizeof(TempHash));
	tempHash->memHash = (char*)malloc(arg->task.memory);
	tempHash->pointerMem = 0;
	tempHash->filename = (char*)malloc(100);
        sprintf(tempHash->filename,"./fileHash.%lu-%lu",arg->handle.thread, getpid());
	tempHash->fileHash = fopen(tempHash->filename , "w");
	tempHash->pointerFile = 0 ;
	tempHash->start = time(NULL);

	while(arg->handle.interrupt == DONT_INTERRUPT_TASK){
		int resultSearch = searchHash(&result, &(arg->task), tempHash);
		if(resultSearch == 1){
			result.status = TASK_SUCCESS;
			sendResult(arg, &result);
			closeThread(tempHash);
			return;
		} else if(resultSearch == -1){
			result.status = TASK_COMPLETE;
			sendResult(arg, &result);
			closeThread(tempHash);
			return;
		}
		timeTranscured = difftime(time(NULL),tempHash->start);

		if (arg->task.cpu <= ((long)timeTranscured)){
			arg->handle.interrupt = INTERRUPT_TASK_SEND_FAIL;
		}
		incrementValue(result.value, arg->task.length);
	}
	if (arg->handle.interrupt == INTERRUPT_TASK_SEND_FAIL){
		result.status = TASK_FAIL;
		sendResult(arg, &result);
		closeThread(tempHash);
	}else if (arg->handle.interrupt == INTERRUPT_TASK_DONT_SEND_FAIL){
		closeThread(tempHash);
	}

}

char *itoaLocal(long value)
 {
     static char buffer[64];       
     long original = value;        

     long c = sizeof(buffer)-1;

     buffer[c] = 0;                

     if (value < 0)                
         value = -value;

     do                            
     {
         buffer[--c] = (value % 10) + '0';
         value /= 10;
     } while (value);

     if (original < 0)
         buffer[--c] = '-';

     return &buffer[c];
 }

