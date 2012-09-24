#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct message_share {
    int value;
    int next;
} MessageShare;

typedef struct sharedMemory {
    pthread_cond_t condToDC;
    pthread_mutex_t mutexToDC;
    int queueToDC;

    int read;
    int write;

    pthread_cond_t consumerToNodo;
    pthread_cond_t enqueueToNodo;
    pthread_mutex_t mutexToNodo;
    MessageShare* queueToNodo;
} SharedMemory;

char* filename;

void* get_shared_mem( size_t size, int id) {
    key_t semkey = ftok(filename, id);
    if (semkey == -1) {
        perror("ftok");
        exit(1);
    }

    int shmidMut = shmget( semkey, size, 0644 | IPC_CREAT);
    
    if (shmidMut == -1) {
        perror("shmget");
        exit(1);
    }
    
    return shmat(shmidMut, (void *)0, 0);
/*
    memset(result, 0, sizeof (SharedMemory));
*/

}

int main(int argc, char** argv) {
    filename = argv[0];
    pid_t childpid;
    int shmidMut;

    childpid = fork();

    if (childpid < 0) {
        perror("Fork Failed");
        exit(1);
    } else if (childpid == 0) {
        SharedMemory* sharedMem = (SharedMemory*) get_shared_mem( sizeof(SharedMemory) , 1);
        
        pthread_mutexattr_t mattr;
        pthread_condattr_t cattr;
        pthread_cond_init(&(sharedMem->condToDC), &cattr);
        pthread_mutex_init(&(sharedMem->mutexToDC), &mattr);

        pthread_mutexattr_init(&mattr);
        pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
        pthread_condattr_init(&cattr);
        pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
        sleep(2);
        sharedMem->queueToDC = NULL;
        sharedMem->read = 2;
        sharedMem->write = 2;

        
/*
        strncpy(argv[0], "DC", strlen(argv[0]) );
*/
        while (1) {
            fprintf(stderr, "Hijo Loop\n");
            pthread_mutex_lock(&sharedMem->mutexToDC);
            if (sharedMem->queueToDC == NULL) {
                fprintf(stderr, "Hijo Wait Cond\n");
                pthread_cond_wait(&sharedMem->condToDC, &sharedMem->mutexToDC);
            }
            fprintf(stderr, "Hijo Empiezo a leer mensaje Mensaje (%d)\n", sharedMem->queueToDC);

            MessageShare* message = (MessageShare*) get_shared_mem( sizeof(MessageShare) , (sharedMem->read)++ );

            fprintf(stderr, "Hijo Empiezo a leer mensaje Mensaje\n");
/*
            sharedMem->queueToDC = message->next;
*/
            fprintf(stderr, "Hijo Lei Mensaje %d\n", message->value);
            //shmdt(message);
            pthread_mutex_unlock(&sharedMem->mutexToDC);
            sleep(5);
        }
    } else {
        sleep(2);
        SharedMemory* sharedMem = (SharedMemory*) get_shared_mem( sizeof(SharedMemory) , 1);

/*
        strncpy(argv[0], "Nodo", strlen(argv[0]) );
*/
        int i = 0;
        while (1) {
            fprintf(stderr, "Padre Loop\n");
            pthread_mutex_lock(&sharedMem->mutexToDC);
            
            MessageShare* message = (MessageShare*) get_shared_mem( sizeof(MessageShare), sharedMem->write);
            message->value = i++;
            message->next = ++(sharedMem->write);
            
            fprintf(stderr, "Padre Agrego Mensaje (%d)\n", message);
/*
            if (sharedMem->queueToDC == NULL) {
                sharedMem->queueToDC = shmidMut;
            } else {
                MessageShare* searchLast = (MessageShare*) shmat(sharedMem->queueToDC, NULL, 0);
                while (searchLast->next != NULL) {
                    searchLast = (MessageShare*) shmat(searchLast->next, NULL, 0);
                }
                searchLast->next = shmidMut;
            }
*/

            fprintf(stderr, "Padre intenta enviar Cond %d   %d\n", (sharedMem->condToDC), &(sharedMem->condToDC));

            pthread_cond_signal(&(sharedMem->condToDC));
            fprintf(stderr, "Padre Cond Enviado\n");
            pthread_mutex_unlock(&sharedMem->mutexToDC);
            sleep(1);
        }
    }

    /*
            shmdt(sharedMem);
     */
    sleep(10);
    return 0;
}
