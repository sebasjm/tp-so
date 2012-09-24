#include "resolvers.h"
#include "messages/interface-daemon.h"
#include "xml/ansi/configuration.h"

#include <iostream>
#include <ipc/socket.h>
#include <ipc/socketBuilder.h>
#include <messages/daemon-daemon.h>
#include <node/daemon/handshake.h>
#include <messages/webserver-buffer.h>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <xml/configuration.h>

#include <iterator>
#include <algorithm>

#include <util/template.h>
#include <ipc/webConnector.h>
#include "db.h"

using namespace std;

#include <util/log.h>
__init_log(webserver_resolvers)

char* get_param(vector<string> params, char* param_name);
char* job_to_string(JobClass* job);
bool el_job_esta_en_ejecucion(char*);

char* dc_ip = NULL;
char* dc_puerto = NULL;

HANDLE h2 = NULL;

HANDLE get_handle2(){
	if (h2 == NULL) h2 = HeapCreate(0,0,0);
	return h2;
}


char* lista_de_jobs_to_string(char* main, char* tmpl, cursor_list* list) {
    char* resultado = replace_var(main,"jobs",  "${job_node}${job_next}");

    cursor_list* ptr = list;

    while(ptr) {
        char* str = replace_var( tmpl,"id", ptr->key);
        str = dynamic_replace_var(str,"desc", ptr->value );

        resultado = dynamic_replace_var(resultado,"job_node", str );
        resultado = dynamic_replace_var(resultado,"job_next", "${job_node}${job_next}" );
        ptr = ptr->next;

        free(str);
    }

    resultado = dynamic_replace_var(resultado, "job_node", "");
    resultado = dynamic_replace_var(resultado, "job_next", "");
    return resultado;
}


void resolver_root    (SocketClass<web_line> sock, url_info u_info) {
    info << " query: pagina root" << endl;
    web_line linea = {
        "<html> \
        <body> \
            Pagina root \
        </body> \
        </html> \n"
    };
    sock << linea;
}

void resolver_register(SocketClass<web_line> sock, url_info urlinfo) {
    info << " query: registracion " << endl;
    dc_ip = get_param(urlinfo.request_params, "ip=");
    dc_puerto = get_param(urlinfo.request_params, "puerto=");
    if ( dc_ip != NULL && dc_puerto != NULL )
        info << " nuevo dc registrado ip = " << dc_ip << " puerto " << dc_puerto << endl;
}

void resolver_status  (SocketClass<web_line> sock_web, url_info urlinfo) {
    info << " query: status " << endl;
    if (dc_ip == NULL || dc_puerto == NULL) return;
    if (!strcmp("",dc_ip) || !strcmp("",dc_puerto)) return;
    
    char main_tem[] =
        "<html> \
        <body> \
            System Status ---- [Tu ip es ${ip}, gran hermano te vigila] \
            <hr/>\
            Nombre del job: ${job_name} <br/>\
            Cantidad de Jobs en ejecucion: ${cant_job} <br/> \
            Cantidad de tareas por Job: ${cant_task} <br/> \
            Cantidad de tareas bloqueadas: ${blocked_task} <br/> \
            Cantidad de tareas completas: ${completed_task} <br/> \
            Cantidad de tareas en espera: ${wait_task} <br/> \
            Cantidad de tareas ejecutandose: ${running_task} <br/> \
        </body> \
        </html> \n"
    ;

    char* resultado = NULL;

    SocketBuilderClass* builder = new SocketBuilderClass();
    builder->ip( dc_ip );
    builder->puerto( dc_puerto );
    SocketClass<>* sock = (SocketClass<>*) builder->startConnection();
    
    debug << " inciando conexion con dc " << endl;
    if (sock != NULL && client_handshake(sock)) {
        debug << " conexion ok " << endl;
        SocketClass<insomnio_header> sock_header(sock->id());
        SocketClass<info_nodo> sock_info(sock->id());
        SocketClass<nombre_nodo> sock_nombre(sock->id());
        insomnio_header header = {"",101,0,0,0}; //TYPE_DISPLAY -- si cambia de valor, cambiarlo aca
        sock_header << header;

        info_nodo listo_el_pollo = {"", "", "", "", 0, 0, 0, 0, 0};
        nombre_nodo pelada_la_gallina = {""};

        sock_info >> listo_el_pollo; // info del nodo propia

        int es_dc = listo_el_pollo.dc;

        //info de entrantes
        do{ sock_info >> listo_el_pollo; } while (strcmp(listo_el_pollo.nombre, "listo"));
        //info de salientes
        do{ sock_info >> listo_el_pollo; } while (strcmp(listo_el_pollo.nombre, "listo"));
        //info de red
        do{ sock_nombre >> pelada_la_gallina; } while (strcmp(pelada_la_gallina.id, "listo"));

        debug << "vamos a lo que me importa... la info de DC " << endl;
        //info que me importa
        if (es_dc) {
            char numeros[10];
            SocketClass<DCState> sock_state(sock->id());
            DCState state;
            sock_state >> state;
            resultado = replace_var(main_tem,"ip",urlinfo.ip);
            resultado = dynamic_replace_var(resultado,"job_name",state.job_id);
            sprintf(numeros,"%d",state.cant_jobs);
            resultado = dynamic_replace_var(resultado,"cant_job",numeros);
            sprintf(numeros,"%d",state.cant_task);
            resultado = dynamic_replace_var(resultado,"cant_task",numeros);
            sprintf(numeros,"%d",state.cant_wait_task);
            resultado = dynamic_replace_var(resultado,"wait_task",numeros);
            sprintf(numeros,"%d",state.cant_running_task);
            resultado = dynamic_replace_var(resultado,"running_task",numeros);
            sprintf(numeros,"%d",state.cant_blocked_task);
            resultado = dynamic_replace_var(resultado,"blocked_task",numeros);
            sprintf(numeros,"%d",state.cant_completed_task);
            resultado = dynamic_replace_var(resultado,"completed_task",numeros);
            
            web_send(sock_web, resultado);
            free(resultado);
        } else {
            error << "El nodo registrado no es DC - ip " << dc_ip << " puerto " << dc_puerto << endl;
            char error_tem[] =
                    "<html> \
                    <body> \
                        El nodo registrado no es DC <br/> \
                        IP: ${ip} Puerto: ${puerto} <br/> \
                    </body> \
                    </html> \n"
                    ;
            resultado = replace_var(error_tem, "ip", dc_ip);
            resultado = dynamic_replace_var(resultado, "puerto", dc_puerto);
            web_send(sock_web, resultado);
            free(resultado);
        }

//        sock_header.stop();
    } else {
        error << "No me pude conectar con el DC ip " << dc_ip << " puerto " << dc_puerto << endl;
        char error_tem[] =
                "<html> \
        <body> \
            No me pude conectar con el DC <br/> \
            IP: ${ip} Puerto: ${puerto} <br/> \
        </body> \
        </html> \n"
        ;
        resultado = replace_var(error_tem, "ip", dc_ip);
        resultado = dynamic_replace_var(resultado, "puerto", dc_puerto);
        web_send(sock_web, resultado);
        free(resultado);
    }
    if (sock != NULL) {
        sock->stop();
        delete sock;
    }
    delete builder;


}

char* job_to_string(JobClass* itJob) {
    web_line job_template = {
        "| <job id=\"${job_id}\"> \n\
            <target> \n\
                <hash>${job_target_hash}</hash> \n\
                <method>${job_target_method}</method> \n\
                <length>${job_target_length}</length> \n\
            </target> \n\
            <tasks> \n\
                ${job_tasks} \n\
            </tasks> \n\
        </job>\n"
    };
    web_line task_template = {
        "<task> \n\
            <task-id>${task_id}</task-id> \n\
            <task-description>${task_desc}</task-description> \n\
            <resources> \n\
                ${task_resources} \n\
            </resources> \n\
            <dependencies> \n\
                ${task_deps} \n\
            </dependencies> \n\
        </task>\n                "
    };
    web_line template_resource = {
        "<resource name=\"${resource_name}\"   value=\"${resource_value}\" />\n                "
    };
    web_line template_dependency = {
        "<dependency ref=\"${dependency_name}\" /> \n                "
    };

    char entero[50];
    sprintf(entero,"%d",itJob->target()->length());

    char* resultado = replace_var(job_template.str,"a", "a" );
    resultado = dynamic_replace_var(resultado,"job_id", itJob->id()->data() );
    resultado = dynamic_replace_var(resultado,"job_target_hash", itJob->target()->hash().data() );
    resultado = dynamic_replace_var(resultado,"job_target_method", itJob->target()->method().data() );
    resultado = dynamic_replace_var(resultado,"job_target_length", entero );

    resultado = dynamic_replace_var(resultado,"job_tasks", "${job_node}${job_next}" );
    for (list<TaskClass*>::iterator itTask = itJob->tasks()->begin(); itTask != itJob->tasks()->end(); itTask++) {
        resultado = dynamic_replace_var(resultado,"job_node", task_template.str);

        resultado = dynamic_replace_var(resultado,"task_id", (*itTask)->taskId()->data() );
        resultado = dynamic_replace_var(resultado,"task_desc", (*itTask)->taskDescription()->data() );

        resultado = dynamic_replace_var(resultado,"task_deps", "${task_node}${task_next}" );
        for (list<DependencyClass*>::iterator itDep = (*itTask)->dependencies()->begin(); itDep != (*itTask)->dependencies()->end(); itDep++) {
            resultado = dynamic_replace_var(resultado,"task_node",template_dependency.str);

            resultado = dynamic_replace_var(resultado,"dependency_name", (*itDep)->nombre().data() );

            resultado = dynamic_replace_var(resultado,"task_next", "${task_node}${task_next}" );
        }
        resultado = dynamic_replace_var(resultado,"task_node", "" );
        resultado = dynamic_replace_var(resultado,"task_next", "" );

        resultado = dynamic_replace_var(resultado,"task_resources", "${task_node}${task_next}" );
        for (list<ResourceClass*>::iterator itRes = (*itTask)->resources()->begin(); itRes != (*itTask)->resources()->end(); itRes++) {
            resultado = dynamic_replace_var(resultado,"task_node",template_resource.str);

            resultado = dynamic_replace_var(resultado,"resource_name", (*itRes)->nombre().data() );
            resultado = dynamic_replace_var(resultado,"resource_value", (*itRes)->value().data() );

            resultado = dynamic_replace_var(resultado,"task_next", "${task_node}${task_next}" );
        }
        resultado = dynamic_replace_var(resultado,"task_node", "" );
        resultado = dynamic_replace_var(resultado,"task_next", "" );

        resultado = dynamic_replace_var(resultado,"job_next", "${job_node}${job_next}" );
    }
    resultado = dynamic_replace_var(resultado,"job_node", "" );
    resultado = dynamic_replace_var(resultado,"job_next", "" );
    return resultado;
}


void resolver_ready   (SocketClass<web_line> sock, url_info u_info) {
    info << " query: ready queue " << endl;
    stringstream result( stringstream::in | stringstream::out );

    //parece inutil pero sirve para inicializar
    for (list<JobClass*>::iterator itJob = u_info.ready_queue->begin(); itJob != u_info.ready_queue->end(); itJob++) {
        char* resultado = job_to_string(*itJob);
        web_send(sock, resultado);
        free(resultado);
    }
    
}

void resolver_done    (SocketClass<web_line> sock, url_info urlinfo) {
    info << " query: done queue " << endl;
    char main_template[] = {
        "<html> \
        <body> \
            Jobs terminados en la base de datos doneQueue <br/> \
            ${jobs}\
        </body> \
        </html> \n"
    };
    
    char* jobid = get_param(urlinfo.request_params, "jobid=");
    char* clave = get_param(urlinfo.request_params, "clave=");

    if (jobid != NULL && clave != NULL && strcmp("",clave) && strcmp("",jobid) ) {
        info << " job " << jobid << " clave " << clave << endl;
        for (list<JobClass*>::iterator itJob = urlinfo.ready_queue->begin(); itJob != urlinfo.ready_queue->end(); itJob++) {
            if (!strcmp((*itJob)->id()->data(), jobid)) {
                db_done_set_value(jobid, clave);
                urlinfo.ready_queue->erase(itJob);
                break;
            }
        }
    }
    
    cursor_list* list = db_done_get_all();
    char* resultado = lista_de_jobs_to_string(main_template,"Job id: ${id} <br/> Resultado: ${desc} <br/>", list);
    
    web_send(sock, resultado);
    free_cursor_list( list );
    free(resultado);
}

void resolver_error   (SocketClass<web_line> sock, url_info urlinfo) {
    info << " query: error queue " << endl;
    char main_template[] = {
        "<html> \
        <body> \
            Jobs que no terminaron bien en la base de datos errorQueue <br/> \
            ${jobs}\
        </body> \
        </html> \n"
    };
    char* jobid = get_param(urlinfo.request_params, "jobid=");
    char* razon = get_param(urlinfo.request_params, "razon=");

    if (jobid != NULL && razon != NULL && strcmp("",razon) && strcmp("",jobid) ) {
        info << " job " << jobid << " razon " << razon << endl;
        for (list<JobClass*>::iterator itJob = urlinfo.ready_queue->begin(); itJob != urlinfo.ready_queue->end(); itJob++) {
            if (!strcmp((*itJob)->id()->data(), jobid)) {
                db_error_set_value(jobid, razon);
                urlinfo.ready_queue->erase(itJob);
                break;
            }
        }
    }
    cursor_list* list = db_error_get_all();
    char* resultado = lista_de_jobs_to_string(main_template,"Job id: ${id} <br/> Razon: ${desc} <br/>", list);

    web_send(sock, resultado);
    free_cursor_list( list );
    free(resultado);
}

void resolver_result  (SocketClass<web_line> sock, url_info urlinfo) {
    
    char* jobid = get_param(urlinfo.request_params, "jobid=");

    char* resultado = NULL;

    if (jobid == NULL || !strcmp("",jobid) ) {
        info << " query: result general " << endl;
        char str[] = {
            "<html> \
            <body> \
                ${jobs_error}\
                <hr/> \
                ${jobs_done}\
            </body> \
            </html> \n"
        };
        char error_str[] = {
            " Lista de Jobs con error <br/>\n\
                ${jobs}\
            \n"
        };
        char done_str[] = {
            " Lista de Jobs terminados <br/>\n\
                ${jobs}\
            \n"
        };
        cursor_list* error_list = db_error_get_all();
        cursor_list* done_list  = db_done_get_all();
        
        char* error_resultado = lista_de_jobs_to_string(error_str,"Job id: ${id} <br/> Razon: ${desc} <br/>", error_list);
        char* done_resultado  = lista_de_jobs_to_string(done_str, "Job id: ${id} <br/> Clave: ${desc} <br/>", done_list);

        resultado = replace_var( str,"jobs_error", error_resultado );
        resultado = dynamic_replace_var(resultado,"jobs_done", done_resultado );

        free_cursor_list( error_list );
        free_cursor_list( done_list );

    } else {
        info << " query: result para el job " << jobid << endl;
        char str[] = {
            "<html> \
            <body> \
                ${jobs}\
            </body> \
            </html> \n"
        };

        char* str_job = db_done_get_value( jobid );

        debug << "str_job " << (str_job?str_job:"null") << endl;

        if ( !str_job || !strcmp("",str_job) ) {
            if (str_job) HeapFree(get_handle(),0,str_job);
            str_job = db_error_get_value(jobid);
        }

        debug << "str_job " << (str_job?str_job:"null") << endl;

        if ( !str_job || !strcmp("",str_job) ) {
            if (str_job) HeapFree(get_handle(),0,str_job);
            if ( el_job_esta_en_ejecucion(jobid) ) {
                str_job = "Esta siendo ejecutada en el grid, verificar status";
            };
        }

        debug << "str_job " << (str_job?str_job:"null") << endl;

        if ( !str_job || !strcmp("",str_job) ) {
			str_job = (char*) HeapAlloc(get_handle2(),0,500);
            strcpy(str_job,"no pude encontrar el job!");
        }

        debug << "str_job " << (str_job?str_job:"null") << endl;

        resultado = replace_var( str,"jobs", str_job );
        if (str_job) HeapFree(get_handle2(),0,str_job);
    }

    web_send(sock, resultado);
    free(resultado);
}

void resolver_server_header(SocketClass<web_line> sock, url_info info) {
    web_line linea = {
        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \
            \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"> \
        \n"
    };
    sock << linea;
}

void resolver_server_404(SocketClass<web_line> sock, url_info url_linfo) {
    info << " query: 404 " << endl;
    web_line mensaje_de_error = {
        "<html> \
        <body> \
            La pagina solicitada no esta disponible \
        </body> \
        </html> \n"
    };
    sock << mensaje_de_error;
}

char* get_param(vector<string> params, char* param_name) {
    for (vector<string>::iterator i = params.begin(); i != params.end(); i++ ) {
        int len = strlen( param_name );
        if ( !strncmp(param_name,i->data(), len ) ) {
			int param_len = strlen(&(i->data()[len]));
			char* result = (char*)HeapAlloc(get_handle2(),0,param_len+1);
			memcpy(result,&(i->data()[len]),param_len+1);
			return result;
        }
    }
    return "";
}

bool el_job_esta_en_ejecucion(char* jobid) {
    bool result = false;
    
    SocketBuilderClass* builder = new SocketBuilderClass();
    builder->ip( dc_ip );
    builder->puerto( dc_puerto );
    SocketClass<>* sock = (SocketClass<>*) builder->startConnection();

    debug << " inciando conexion con dc " << endl;
    if (sock != NULL && client_handshake(sock)) {
        debug << " conexion ok " << endl;
        SocketClass<insomnio_header> sock_header(sock->id());
        SocketClass<info_nodo> sock_info(sock->id());
        SocketClass<nombre_nodo> sock_nombre(sock->id());
        insomnio_header header = {"",101,0,0,0}; //TYPE_DISPLAY -- si cambia de valor, cambiarlo aca
        sock_header << header;

        info_nodo listo_el_pollo = {"", "", "", "", 0, 0, 0, 0, 0};
        nombre_nodo pelada_la_gallina = {""};

        sock_info >> listo_el_pollo; // info del nodo propia

        int es_dc = listo_el_pollo.dc;

        //info de entrantes
        do{ sock_info >> listo_el_pollo; } while (strcmp(listo_el_pollo.nombre, "listo"));
        //info de salientes
        do{ sock_info >> listo_el_pollo; } while (strcmp(listo_el_pollo.nombre, "listo"));
        //info de red
        do{ sock_nombre >> pelada_la_gallina; } while (strcmp(pelada_la_gallina.id, "listo"));

        debug << "vamos a lo que me importa... la info de DC " << endl;
        //info que me importa
        if (es_dc) {
            SocketClass<DCState> sock_state(sock->id());
            DCState state;
            sock_state >> state;
            result = !strcmp(state.job_id,jobid);
        }
    }
    delete builder;
    if (sock) {
        sock->stop();
        delete sock;
    }
    return result;
}
