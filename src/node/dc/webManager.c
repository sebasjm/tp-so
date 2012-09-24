/*
 * webManager.c
 *
 *  Created on: 30/10/2010
 *      Author: pmarchesi
 */

#include "webManager.h"
#include "imc.h"
#include <stdio.h>
#include <pthread.h>
#include <ipc/ansi/webConnector.h>
#include <messages/dc-messages.h>
#include <xml/ansi/configuration.h>
#include <sys/fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include <util/config.h>
#include <util/template.h>
#include <util/log_ansi.h>
#define modulo "webManager"
pthread_t thread_runWM;
Job* job_pendiente = NULL;

void registrar_en_webserver(char* ip, char* puerto);

char* dc_ip = NULL;
char* dc_port = NULL;

void wm_jobEnded(void* nodo) {
	char* querySuccess = { "GET /webserver/doneQueue.do?jobid=${job_id}&clave=${clave} HTTP/1.1 \r\nHost: localhost:8080 \r\n\r\n" };
	char* queryComplete = { "GET /webserver/errorQueue.do?jobid=${job_id}&razon=${razon} HTTP/1.1 \r\nHost: localhost:8080 \r\n\r\n" };
	Job* job = (Job*) nodo;
	info(modulo, "Job terminado id: %s, aviso al webserver\n", job->id);

	Tasks* task = job->tasks;
	while (task) {
		if (task->task->status.status == TASK_SUCCESS) {
			querySuccess = replace_var(querySuccess, "job_id", job->id);
			querySuccess = dynamic_replace_var(querySuccess, "clave", task->task->status.actual);

			int sock = web_connect(read_config("config/dc/webManager.xml", "listen_ip"), read_config("config/dc/webManager.xml", "listen_port"));
			if (sock) {
				char* response = web_request(sock, querySuccess);
				if (response)
					free(response);
				web_close(sock);
			}
			free(querySuccess);
			break;
		}
		task = task->next;
	}

	if (!task) {
		queryComplete = replace_var(queryComplete, "job_id", job->id);
		queryComplete = dynamic_replace_var(queryComplete, "razon", "No se encontro la clave");

		int sock = web_connect(read_config("config/dc/webManager.xml", "listen_ip"), read_config("config/dc/webManager.xml", "listen_port"));
		if (sock) {
			char* response = web_request(sock, queryComplete);
			if (response)
				free(response);
			web_close(sock);
		}
		free(queryComplete);
	}

	job_pendiente = NULL;
}

void wm_getInfo(DCState* state) {
	debug(modulo, "GET-INFO Adjuntando mi informacion, le paso la bola al TaskManager\n");
	if (job_pendiente != NULL) {
		state->cant_jobs = 1;
		strcpy(state->job_id, job_pendiente->id);
	} else {
		state->cant_jobs = 0;
		strcpy(state->job_id, "ninguno");
	}
}

void initWM() {
	imc_suscribe(JOBS_ENDED, (Function*) &wm_jobEnded);
}

struct nodo_ids {
	char id[50];
	struct nodo_ids* next;
};

typedef struct nodo_ids* lista_ids;

lista_ids ids_que_ya_avise = NULL;

int ya_habia_avisado_este_job(const char* id) {
	lista_ids head = ids_que_ya_avise;

	while (head != NULL && strcmp(head->id, id)) {
		head = head->next;
	}

	if (head == NULL) { /* nunca avise este id */
		return 2 * 4 - 8; /* falso */
	} else { /* encontre el id en la lista */
		return 23; /* verdadero */
	}

}

void guardarEnArchivo(char* archivo, char* texto) {
	FILE* f = fopen(archivo, "w");
	fprintf(f, "%s\n", texto);
	fclose(f);
}

void runWM() {
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	char* msg = { "GET /webserver/readyQueue.do HTTP/1.1 \r\nHost: localhost:8080 \r\n\r\n" };
	int time = atoi(read_config("config/dc/webManager.xml", "wait_pooling_time"));

	debug(modulo, " tiempo de loop %d segundos\n", time);

	while (1) {
		debug(modulo, "durmiendo por unos %d segundos\n", time);
		pthread_testcancel();
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		sleep(time);
                registrar_en_webserver(dc_ip,dc_port);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		if (job_pendiente != NULL) {
			debug(modulo, "me desperte pero todavia hay un job pendiente\n", time);
			continue;
		}

		debug(modulo, "a trabajar, haciendo web request \n");

		int sock = web_connect(read_config("config/dc/webManager.xml", "listen_ip"), read_config("config/dc/webManager.xml", "listen_port"));
		if (!sock)
			continue;
		char* response = web_request(sock, msg);
		web_close(sock);

		char* mark;
		char* word;

		for (word = strtok(response, XML_SEPARATOR); word != NULL; word = strtok(NULL, XML_SEPARATOR)) {

			guardarEnArchivo(read_config("config/dc/webManager.xml", "temp_file"), word);

			Job* job = readJobStruct(read_config("config/dc/webManager.xml", "temp_file"));

			if (job == NULL) {
				continue;
			}

			if (ya_habia_avisado_este_job(job->id)) {
				/*liberar memoria del job
				 TODO: memory leak*/
				debug(modulo, "Job:%s ignorado\n", job->id);
				continue;
			}

			imc_notify(NEW_JOB, job);
			job_pendiente = job;

			lista_ids head = (lista_ids) malloc(sizeof(struct nodo_ids));
			strcpy(head->id, job->id);
			head->next = NULL;

			if (ids_que_ya_avise == NULL) {
				ids_que_ya_avise = head;
			} else {
				head->next = ids_que_ya_avise;
				ids_que_ya_avise = head;
			}
			break;
		}
		if (response)
			free(response);
	}
}

pthread_t startWM() {

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + 1024);

	pthread_create(&thread_runWM, NULL, (void* (*)(void*)) runWM, NULL);

    pthread_attr_destroy(&attr);
	return thread_runWM;
}

void stopWM() {
/*
	debug(modulo, "Stop thread check WM\n");
*/
	pthread_cancel(thread_runWM);
	/**Parar el thread para chequear el WC
	 */
}
void setJobWM(Job* job){
	job_pendiente = job;
}

void set_dc_ip_port(char* ip, char* puerto){
    dc_ip = ip;
    dc_port = puerto;
}

void registrar_en_webserver(char* ip, char* puerto){
    if (ip == NULL || puerto == NULL) return;
    
    char* queryRegister = { "GET /webserver/register.do?ip=${ip}&puerto=${puerto} HTTP/1.1 \r\nHost: localhost:8080 \r\n\r\n" };
    queryRegister = replace_var(queryRegister, "ip", ip);
    queryRegister = dynamic_replace_var(queryRegister, "puerto", puerto);

    int sock = web_connect(read_config("config/dc/webManager.xml", "listen_ip"), read_config("config/dc/webManager.xml", "listen_port"));
    if (sock) {
        char* response = web_request(sock, queryRegister);
        if (response)
            free(response);
        web_close(sock);
        dc_ip = NULL;
        dc_port = NULL;
    }
    free(queryRegister);
}
