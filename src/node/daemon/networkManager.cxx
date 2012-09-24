#include <iostream>
#include <stdlib.h>
#include <ipc/socket.h>
#include <ipc/socketBuilder.h>
#include <ipc/socketEvent.h>
#include <ipc/listener.h>
#include "handshake.h"
#include <util/config.h>
#include "networkManager.h"
#include "netDiscovery.h"
#include "migration.h"
#include "admTask.h"
#include "commands.h"
#include "ipc/ipcDaemon.h"
#include <string.h>
#include <util/weighted.h>
#include <sys/wait.h>

#include <node/daemon/common.h>

using namespace std;

info_red* la_red_;
char* nodoName;
char* configName;
int connectionDC = 0;
info_nodo* viejoDC = NULL;
Lala* memDc = NULL;

typedef struct {
    int prox_paso;
    insomnio_header* header;
    void* buf;
} estado_conexion;

estado_conexion last_headers[255];

Listener* listener = NULL;
//soy_dc y nunca_va_a_tener_saliente son parecidos pero no iguales
bool nunca_va_a_tener_saliente = false;
bool soy_dcs = false;

#include <util/log.h>
__init_log(networkManager)

int conectar_a_nodo(char* nodoDestino) {
    debug << "conectando nodo " << nodoDestino << endl;
	nodo_red* nodo = buscar_nodo_en_red(nodoDestino);
	if (nodo == NULL) {
		return 0;
	}
	debug << "conectar a " << nodo->info.nombre << " " << nodo->info.conexion << " " << nodo->info.ip << ":" << nodo->info.puerto << endl;
	//si no es vecino directo, genero una conexion
	if (nodo->info.conexion == 0) {
		SocketBuilderClass* builder = new SocketBuilderClass();
		builder->ip(nodo->info.ip);
		builder->puerto(nodo->info.puerto);
		SocketClass<>* sock = (SocketClass<>*) builder->startConnection();

		if (sock != NULL && client_handshake(sock)) {
//			error << "OK: " << nodo->info.nombre << " " << sock->id() << endl;
			nodo->info.conexion = sock->id();
		}

		if (sock != NULL)
			delete sock;
		delete builder;
	}

	return nodo->info.conexion;
}
int conectar_a_nodo_udp(char* nodoDestino) {
	nodo_red* nodo = buscar_nodo_en_red(nodoDestino);
	if (nodo == NULL) {
		return 0;
	}
	debug << "conectar a " << nodo->info.nombre << " " << nodo->info.conexion << " " << nodo->info.ip << ":" << nodo->info.puerto << endl;
	//si no es vecino directo, genero una conexion
	if (nodo->info.conexion == 0) {
		SocketBuilderClass* builder = new SocketBuilderClass();
		builder->ip(nodo->info.ip);
		builder->puerto(nodo->info.puerto);
		SocketClass<>* sock = (SocketClass<>*) builder->startUnconnected();

		if (sock != NULL) {
			nodo->info.conexion = sock->id();
		}

		if (sock != NULL)
			delete sock;
		delete builder;
	}

	return nodo->info.conexion;
}
void desconectar_a_nodo(char* nodoDestino) {
	nodo_red* nodo = buscar_nodo_en_red(nodoDestino);

	//si es vecino directo no cierra la conexion
	if (nodo->info.distancia == 1) {
		return;
	}

	SocketClass<> sock(nodo->info.conexion);
	sock.stop();
	nodo->info.conexion = 0;
}

bool enviar_ping(char* nodoDestino, info_nodo info_ping, insomnio_header header) {
	int conexion = conectar_a_nodo(nodoDestino);

	if (!conexion)
		return false;

        debug << " enviando info destino a " << nodoDestino << " id " << conexion << " header payload " << header.payload << endl;

	SocketClass<insomnio_header> sock_header(conexion);
	SocketClass<msg_red> sock_msg(conexion);

	nombre_nodo name;
	sprintf(name.id, "%s", nodoName);

	msg_red msgRed;
	msgRed.nombre = name;
	msgRed.info = info_ping;

	sock_header << header;
	sock_msg << msgRed;

//	desconectar_a_nodo(nodoDestino);
//	info_ping.conexion = conexion;
	return true;
}
int enviar_solo_header(int conexion, insomnio_header header) {
	if (!conexion)
		return conexion;

	SocketClass<insomnio_header> sock_header(conexion);

	sock_header << header;
	return conexion;

}
int enviar_solo_header(char* nodoDestino, insomnio_header header) {
	int conexion = conectar_a_nodo(nodoDestino);

	conexion = enviar_solo_header(conexion, header);

	desconectar_a_nodo(nodoDestino);
	return conexion;
}
int enviar_header_y_nombrenodo(char* nodoDestino, insomnio_header header, nombre_nodo nombre) {
	int conexion = conectar_a_nodo(nodoDestino);

	if (!conexion)
		return conexion;

	SocketClass<insomnio_header> sock_header(conexion);
	SocketClass<nombre_nodo> sock_nombre(conexion);

	sock_header << header;
	sock_nombre << nombre;

	desconectar_a_nodo(nodoDestino);
	return conexion;
}

int enviar_solo_header_no_desconectar(char* nodoDestino, insomnio_header header) {
	int conexion = conectar_a_nodo(nodoDestino);
	if (!conexion)
		return conexion;
	SocketClass<insomnio_header> sock_header(conexion);
	sock_header << header;
	return conexion;
}
int enviar_solo_header_udp(char* nodoDestino, insomnio_header header) {
	int conexion = conectar_a_nodo_udp(nodoDestino);
	if (!conexion)
		return conexion;
	SocketClass<insomnio_header> sock_header(conexion);
	sock_header << header;
	return conexion;
}
int enviar_header_y_nodo_to_dc(int conexion, insomnio_header header, nodo_to_dc* info) {

	if (!conexion)
		return conexion;

	SocketClass<insomnio_header> sock_header(conexion);
	SocketClass<nodo_to_dc> sock_info(conexion);

	sock_header << header;
	sock_info << *info;

	return conexion;
}
int enviar_header_e_info_nodo(int conexion, insomnio_header header, info_nodo* info) {

	if (!conexion)
		return conexion;

	SocketClass<insomnio_header> sock_header(conexion);
	SocketClass<info_nodo> sock_info(conexion);

	sock_header << header;
	sock_info << *info;

	return conexion;
}
int enviar_header_y_info(int conexion, insomnio_header header, info_nodo info) {
	SocketClass<insomnio_header> sock_header(conexion);
	SocketClass<info_nodo> sock_info(conexion);

	sock_header << header;
	sock_info << info;

	return conexion;
}
int enviar_header_y_nodo_dc(char* nodoDestino, insomnio_header header, nodo_to_dc info) {
	int conexion = conectar_a_nodo(nodoDestino);

	if (!conexion)
		return conexion;

	SocketClass<insomnio_header> sock_header(conexion);
	SocketClass<nodo_to_dc> sock_info(conexion);

	sock_header << header;
	sock_info << info;
	desconectar_a_nodo(nodoDestino);
	return 0;
}
int enviar_header_y_info(char* nodoDestino, insomnio_header header, info_nodo info) {
	int conexion = conectar_a_nodo(nodoDestino);

	if (!conexion)
		return conexion;
	int result = enviar_header_y_info(conexion, header, info);
	desconectar_a_nodo(nodoDestino);
	return result;
}
int enviar_header_y_job(int conexion, insomnio_header header, Job* info) {
	if (!conexion)
		return conexion;

	SocketClass<insomnio_header> sock_header(conexion);
	SocketClass<Job> sock_info(conexion);

	sock_header << header;
	sock_info << *info;

	return conexion;
}
int enviar_header_y_task(int conexion, insomnio_header header, Task* info) {
	if (!conexion)
		return conexion;

	SocketClass<insomnio_header> sock_header(conexion);
	SocketClass<Task> sock_info(conexion);

	sock_header << header;
	sock_info << *info;

	return conexion;
}
int enviar_header_y_dep(int conexion, insomnio_header header, DependencyNet* info) {
	if (!conexion)
		return conexion;

	SocketClass<insomnio_header> sock_header(conexion);
	SocketClass<DependencyNet> sock_info(conexion);

	sock_header << header;
	sock_info << *info;

	return conexion;
}
int enviar_header_y_nodo(int conexion, insomnio_header header, NodeWorking* info) {

	if (!conexion)
		return conexion;

	SocketClass<insomnio_header> sock_header(conexion);
	SocketClass<NodeWorking> sock_info(conexion);

	sock_header << header;
	sock_info << *info;

	return conexion;
}

info_nodo* dame_un_nodo_de_distancia_1() {
	info_red* nodos = red_de_nodos();
	if (nodos->nodos_salientes != NULL) {
		debug << "Distancia 1 - nodos salientes" << endl;
		nodo_red* sal = nodos->nodos_salientes;
		while (sal != NULL) {
			if (sal->info.distancia == 1) {
				debug << "Distancia 1 - lo enconte " << sal->info.nombre << endl;
				return &(sal->info);
			}
			debug << "Distancia 1 - no es este " << sal->info.nombre << " dist" << sal->info.distancia << endl;
			sal = sal->next;
		}
	}
	if (nodos->nodos_entrantes != NULL) {
		nodo_red* ent = nodos->nodos_entrantes;
		while (ent != NULL) {
			if (ent->info.distancia == 1) {
				debug << "Distancia 1 - lo enconte " << ent->info.nombre << endl;
				return &(ent->info);
			}
			debug << "Distancia 1 - no es este " << ent->info.nombre << " dist" << ent->info.distancia << endl;
			ent = ent->next;
		}
	}
	return NULL;
}
int enviar_a_dc_solo_header(insomnio_header header) {
	debug << "Conexion local con dc " << connectionDC << endl;
	if (!connectionDC)
		return connectionDC;

	int escrito = sendMessage(connectionDC, &header, NULL, 0);
	debug << "Escrito en el DC " << escrito << " MSJ: " << (header.payload + 0) << endl;
	return connectionDC;
}
int enviar_dc_caido(){
	info_nodo* nodo = dame_un_nodo_de_distancia_1();
	en_eleccion_de_nuevo_DC();
	if (nodo == NULL) {
			error << "ERROR GRAVE!! El DC local se tiene que migrar pero no tengo nodos vecinos";
			return 0;
	}
	info << "Le digo a " << nodo->nombre << " que se cayo el dc" << endl;
	insomnio_header header = { "", TYPE_DC_CAYO, 0, 0, 0 };
	sprintf(header.id, "%s", nodoName);
	debug << "Mensaje a " << nodo->conexion << endl;
	return enviar_solo_header(nodo->nombre, header);
}
int resolver_deseo_de_migracion(int conexion) {
	info_nodo* nodo = dame_un_nodo_de_distancia_1();
	en_eleccion_de_nuevo_DC();
	if (conexion == connectionDC) {
		if (nodo == NULL) {
			error << "El DC local se tiene que migrar pero no tengo nodos vecinos";
			insomnio_header header = { "", TYPE_IPC_DC_MIGRATE_ERROR, 0, 0, 0 };
			sprintf(header.id, "%s", nodoName);
			return enviar_a_dc_solo_header(header);
		}
                info << "Le digo a " << nodo->nombre << " que me migre" << endl;
		insomnio_header header = { "", TYPE_MIGRACION_DC, 0, 0, 0 };
		sprintf(header.id, "%s", nodoName);
		debug << "Mensaje a " << nodo->conexion << endl;
		return enviar_solo_header(nodo->nombre, header);
	} else {
		insomnio_header header = { "", TYPE_MIGRACION_DC, 0, 0, 0 };
		sprintf(header.id, "%s", nodoName);
		debug << "Mensaje a " << nodo->conexion << endl;
		return enviar_solo_header_no_desconectar(nodo->nombre, header);
	}
}
void resolver_conexion_nodo_local(int dc) {
	info << "Se conecto en DC Local " << dc << endl;

	connectionDC = dc;
	insomnio_header header = { "", TYPE_IPC_NEW_NODO, 0, 0, 1 };
	sprintf(header.id, "%s", nodoName);

	SocketClass<insomnio_header> sock_header(dc);
	SocketClass<info_nodo> sock_info(dc);

	sock_header << header;
	sock_info << red_de_nodos()->mi_nodo->info;

	nodo_red* nodo;
	for (nodo = red_de_nodos()->nodos_entrantes; nodo != NULL; nodo = nodo->next) {
		sock_header << header;
		sock_info << nodo->info;
	}

	for (nodo = red_de_nodos()->nodos_salientes; nodo != NULL; nodo = nodo->next) {
		sock_header << header;
		sock_info << nodo->info;
	}

}
void event_startDC(char migration);

void resolver_solicitud_de_memoria(insomnio_header header, int conexion) {
    info << "pedido de MEM " << endl;
	if (conexion == connectionDC) {
		insomnio_header header = { "", TYPE_IPC_DC_GET_MEMORY, 0, 0, 0 };
		sprintf(header.id, "%s", nodoName);
		debug << "Enviar mensaje de Solicitud de memoria a " << viejoDC->nombre << endl;
		enviar_solo_header(viejoDC->nombre, header);
	} else {
		int con = conectar_a_nodo(header.id);
		Lala* toSend = memDc;
		debug << "Hay memoria para enviar? " << (toSend == NULL ? "NO" : "SI") << endl;
		while (toSend != NULL) {
			debug << "while de MEM " << endl;
			debug << "type " << toSend->type << endl;
			switch (toSend->type) {
			case TYPE_IPC_ADD_JOB:
				debug << "Envio Job" << ((Job*) toSend->mem)->id << endl;
				enviar_estado_del_job(con, (Job*) toSend->mem);
				break;
			case TYPE_IPC_ADD_TASK:
				debug << "Envio Task" << ((Task*) toSend->mem)->id << endl;
				enviar_estado_de_la_task(con, (Task*) toSend->mem);
				break;
			case TYPE_IPC_ADD_DEP:
				debug << "Envio DEP" << ((DependencyNet*) toSend->mem)->idDepen << endl;
				enviar_estado_de_la_dependencia(con, (DependencyNet*) toSend->mem);
				break;
			case TYPE_IPC_ADD_NODO_TASK:
				debug << "Envio NODO-Task" << ((NodeWorking*) toSend->mem)->nodo.nombre << endl;
				enviar_estado_nodo_tarea(con, (NodeWorking*) toSend->mem);
				break;
			}
			toSend = toSend->next;
		}
		enviar_migracion_completa(con);
		memDc = NULL;
		debug << "Enviar mensaje de Solicitud de memoria al dc local" << endl;
		//enviar_a_dc_solo_header(header);
	}
}
void enviar_cancelar_tarea(nombre_nodo nombreNodo) {
	debug << "Nodo a cancelar tarea " << nombreNodo.id << "  Mi nombre " << red_de_nodos()->mi_nodo->info.nombre << endl;
	if (strcmp(nombreNodo.id, red_de_nodos()->mi_nodo->info.nombre) == 0) {
		debug << "Le digo a admTask que cancele la tarea" << endl;
		resolver_msg_cancelTask();
	} else {
		insomnio_header header = { "", TYPE_IPC_CANCEL_TASK, 0, 0, 0 };
		sprintf(header.id, "%s", nodoName);
		debug << "En digo a " << nombreNodo.id << " que cancele la tarea" << endl;
		enviar_header_y_nombrenodo(nombreNodo.id, header, nombreNodo);
	}

}
void enviar_tarea_asignada(int conexion, AssignTask* assign) {
    if (strcmp(assign->nodo, red_de_nodos()->mi_nodo->info.nombre) == 0) {
        resolver_msg_newTask(assign);
        return;
    } else {
        int conexion = conectar_a_nodo(assign->nodo);
        if (!conexion) {
            error << "No me puedo conectar al nodo, no le envio la tarea" << endl;
            return;
        }
        insomnio_header header = {"", TYPE_IPC_ASSIGN_TASK, 0, 0, 1};
        sprintf(header.id, "%s", nodoName);
        SocketClass<insomnio_header> sock_header(conexion);
        SocketClass<AssignTask> sock_info(conexion);

        sock_header << header;
        sock_info << *assign;
        
        desconectar_a_nodo(assign->nodo);
    }
//   	if (connectionDC == conexion) {
//		debug << "Me trato de conectar al nodo " << assign->nodo << endl;
//	} else {
//		resolver_msg_newTask(assign);
//	}

}
void enviar_al_dc_un_task_result(TaskResult* result) {
	if (connectionDC == NULL) {
		error << "Me llego un Task Result pero no puedo enviarlo al DC" << endl;
		return;
	}
	insomnio_header header = { "", TYPE_IPC_TASK_RESULT, 0, 0, 1 };
	sprintf(header.id, "%s", nodoName);
	SocketClass<insomnio_header> sock_header(connectionDC);
	SocketClass<TaskResult> sock_info(connectionDC);

	sock_header << header;
	sock_info << *result;

}
void guardar_memoria_de_dc(void* memo, unsigned int size, int type) {
	Lala* tmp = (Lala*) malloc(sizeof(Lala));
	tmp->type = type;
	tmp->mem = memo;
	tmp->size = size;
	tmp->next = NULL;
	if (memDc == NULL) {
		debug << "Agrego a mem " << tmp->type << endl;
		memDc = tmp;
	} else {
		Lala* last = memDc;
		while (last->next != NULL) {
			last = last->next;
		}
		debug << "Agrego a mem " << tmp->type << endl;
		last->next = tmp;
	}
}
/////////////////////////////////////////

class ConectorCliente: public SocketEventClass {
public:
	ConectorCliente(Listener*, char* _configName);
	virtual bool onMessageRecv(PointClass<>* sock);
	virtual void onMessageSent(PointClass<>* sock);
	virtual void onMessageError(PointClass<>* sock);
	virtual pair<PointClass<>*, SocketEventClass*> onNewConnection(SocketGateClass* sock);
};

info_nodo _generar_mi_info(char* configName) {
	info_nodo mi_info;
	mi_info.dc = 0;
	mi_info.distancia = 1;
	mi_info.conexion = 0;
	mi_info.tiempo_sin_pong = TIEMPO_SIN_PONG;
	sprintf(mi_info.ip, "%s", read_config(configName, "mi_ip"));
	sprintf(mi_info.puerto, "%s", read_config(configName, "mi_puerto"));
	sprintf(mi_info.nombre, "%s", read_config(configName, "mi_nombre"));
	sprintf(mi_info.nombre_saliente, "%s", read_config(configName, "nodo_name"));
	char* disco = read_config(configName, "recursos_disco");
	char* memoria = read_config(configName, "recursos_memoria");
	mi_info.disco = strlen(disco) - 2;
	mi_info.disco = getMultiplierUnit(disco + mi_info.disco) * atoi(disco);
	mi_info.memoria = strlen(memoria) - 2;
	mi_info.memoria = getMultiplierUnit(memoria + mi_info.memoria) * atoi(memoria);

	return mi_info;
}

ConectorCliente::ConectorCliente(Listener* _listener, char* _configName) {
	configName = _configName;
	nodoName = read_config(configName, "mi_nombre");
	info << " iniciando nodo " << nodoName << endl;

	listener = _listener;
        estado_conexion ec = {STATUS_LEER_HEADER,NULL,NULL};
	for (int i = 0; i < 255; i++) {
		last_headers[i] = ec;
	}

	la_red_ = new info_red;
	//    la_red_->conexion_saliente = 0;

	la_red_->nodos_salientes = NULL;
	la_red_->nodos_entrantes = NULL;
	la_red_->mi_nodo = new nodo_red;
	la_red_->mi_nodo->next = NULL;
	la_red_->mi_nodo->info = _generar_mi_info(_configName);
	char* la_ip = read_config(configName, "nodo_ip");
	char* el_puerto2 = read_config(configName, "nodo_port");

	int conexion = enviar_ping_nuevo(nodoName, configName, la_ip, el_puerto2);

	// como no se pudo conectar con su saliente nunca
	// lo va a tener, asique no intentes poner a ninguno como saliente
	nunca_va_a_tener_saliente = conexion == 0;
	/*red_de_nodos()->mi_nodo->info.dc = (conexion == 0 ? 1 : 0);*/

	debug << "conexion " << conexion << endl;

	if (conexion) {
		SocketClass<>* sock_saliente = new SocketClass<> (conexion);
		listener->add(sock_saliente, this);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
	} else {
		// LEVANTAR DC
		event_startDC('0');
	}

	activar_refresco_de_red_por_intervalo(configName, red_de_nodos()->mi_nodo->info.ip, red_de_nodos()->mi_nodo->info.puerto);

	info << " inicializacion completa " << endl;
}

nodo_red* get_nodo_segun_conexion_activa(int conexion) {
	nodo_red* nodo = NULL;

	//si no lo encontro, lo busco en las salientes
	for (nodo = red_de_nodos()->nodos_salientes; nodo != NULL && nodo->info.conexion != conexion; nodo = nodo->next) {

	}
        if ( nodo != NULL ) return nodo;

	for (nodo = red_de_nodos()->nodos_entrantes; nodo != NULL && nodo->info.conexion != conexion; nodo = nodo->next) {

	}

	return nodo;
}

/*
 * Verifica el header y trabaja el mensaje
 * Devuelve true si la conexion deberia cerrarse, false si no
 */
void procesar_mensaje(int conexion) {
	switch (last_headers[conexion].header->payload) {
	case TYPE_PING: {
		SocketClass<msg_red> sock_msg(conexion);
		msg_red msgRed;
		sock_msg >> msgRed;
		if (sock_msg.isClosed()) {
                    last_headers[conexion].prox_paso = STATUS_CERRAR_CONEXION;
                    return;
		}
		msgRed.info.conexion = last_headers[conexion].header->hops == 1 ? sock_msg.id() : 0;
                
		resolver_msg_ping(configName, nodoName, msgRed.nombre.id, msgRed.info, *last_headers[conexion].header);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_PONG: {
		SocketClass<msg_red> sock_msg(conexion);
		msg_red msgRed;
		sock_msg >> msgRed;
		if (sock_msg.isClosed()) {
                    last_headers[conexion].prox_paso = STATUS_CERRAR_CONEXION;
                    return;
		}
		msgRed.info.conexion = last_headers[conexion].header->hops == 1 ? sock_msg.id() : 0;

		resolver_msg_pong(nodoName, msgRed.nombre.id, msgRed.info, *last_headers[conexion].header);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_DISPLAY: {
		resolver_display(conexion);
		if (red_de_nodos()->mi_nodo->info.dc) {
                    debug << " me pidieron mi info y soy DC interfaz: " << conexion << endl;
                    insomnio_header header = { "", TYPE_IPC_INFO_DC, 0, 0, 0 };
                    header.ttl = conexion; // peligro: enviando un valor importante en el campo ttl
                    sprintf(header.id, "%s", nodoName);
                    enviar_a_dc_solo_header(header);
		}
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_REFRESH: {
		resolver_refresh(nodoName);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_STOP: {
//		resolver_stop();
            SocketClass<> sock(connectionDC);
            sock.stop();
                listener->stop();
//                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
		/**IPC - DC*/
	case TYPE_DC_CAYO: {
		debug << "Llego mensaje DC_CAYO\n";
		info_nodo* nodo = NULL;
		nodo_red* dcRed = buscar_dc();
		if (dcRed != NULL)
			nodo = &(dcRed->info);
		resolver_caida_de_DC(nodo, '0');
		/*event_startDC('0');*/
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_ELECCION: {
		debug << "Llego mensaje ELECCION\n";
		//		int desconectar = buscar_nodo_en_red_por_sock(conexion)->info.distancia;
		resolver_msg_eleccion(last_headers[conexion].header->id);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_PARTICIPACION: {
		debug << "Llego mensaje PARTICIPACION\n";
		SocketClass<info_nodo> sock_msg(conexion);
		info_nodo nodo;
		sock_msg >> nodo;
		if (sock_msg.isClosed()) {
                    last_headers[conexion].prox_paso = STATUS_CERRAR_CONEXION;
                    return;
		}
//		buscar_nodo_en_red_por_sock(conexion)->info.distancia;
		resolver_msg_participacion(nodo);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_DC_ELECTO: {
		debug << "Llego mensaje ELECTO\n";
		SocketClass<nodo_to_dc> sock_msg(conexion);
		nodo_to_dc nodo;
		sock_msg >> nodo;
		if (sock_msg.isClosed()) {
                    last_headers[conexion].prox_paso = STATUS_CERRAR_CONEXION;
                    return;
		}
		debug << "El ganador fue.. " << nodo.info.nombre << endl;
		resolver_msg_dcElecto(nodo);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_MIGRACION_DC: {
		debug << "Llego mensaje MIGRACION_DC\n";
		info_nodo* nodo = NULL;
		nodo_red* dcRed = buscar_dc();
		if (dcRed != NULL)
			nodo = &(dcRed->info);
		resolver_caida_de_DC(nodo, '1');
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_IPC_I_AM_DC: {
		debug << "Llego mensaje I_AM_DC\n";
		resolver_conexion_nodo_local(conexion);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_IPC_DC_MIGRATE: {
		error << "Llego mensaje DC_MIGRATE DEPRECADO\n";
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_IPC_DC_GET_MEMORY: {
		debug << "Llego mensaje DC_GET_MEMORY\n";
		resolver_solicitud_de_memoria(*last_headers[conexion].header, conexion);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_IPC_INFO_DC_COMPLETE: {
		debug << "Info sobre el DC completa, respondiendo" << endl;
		SocketClass<DCState> sock_msg(conexion);
		DCState dcstate;
		sock_msg >> dcstate;
		if (sock_msg.isClosed()) {
                    last_headers[conexion].prox_paso = STATUS_CERRAR_CONEXION;
                    return;
		}
//		error << " dcstate " << dcstate.job_id << " interfaz : " << dcstate.conexion_con_interfaz << endl;

		SocketClass<DCState> sock_interfaz(dcstate.conexion_con_interfaz);
		sock_interfaz << dcstate;
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
	case TYPE_IPC_ADD_JOB: {
		debug << "Llego mensaje ADD_JOB\n";
		SocketClass<Job> sock_msg(conexion);
		Job* job = (Job*) malloc(sizeof(Job));
		sock_msg >> *job;
		if (sock_msg.isClosed()) {
                    last_headers[conexion].prox_paso = STATUS_CERRAR_CONEXION;
                    return;
		}
		if (conexion == connectionDC)
			guardar_memoria_de_dc(job, sizeof(Job), TYPE_IPC_ADD_JOB);
		else
			enviar_estado_del_job(connectionDC, job);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_IPC_ADD_TASK: {
		debug << "Llego mensaje ADD_TASK\n";
		SocketClass<Task> sock_msg(conexion);
		Task* task = (Task*) malloc(sizeof(Task));
		sock_msg >> *task;
		if (sock_msg.isClosed()) {
                    last_headers[conexion].prox_paso = STATUS_CERRAR_CONEXION;
                    return;
		}
		if (conexion == connectionDC)
			guardar_memoria_de_dc(task, sizeof(Task), TYPE_IPC_ADD_TASK);
		else
			enviar_estado_de_la_task(connectionDC, task);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_IPC_ADD_DEP: {
		debug << "Llego mensaje ADD_DEP\n";
		SocketClass<DependencyNet> sock_msg(conexion);
		DependencyNet* dep = (DependencyNet*) malloc(sizeof(DependencyNet));
		sock_msg >> *dep;
		if (sock_msg.isClosed()) {
                    last_headers[conexion].prox_paso = STATUS_CERRAR_CONEXION;
                    return;
		}
		if (conexion == connectionDC)
			guardar_memoria_de_dc(dep, sizeof(DependencyNet), TYPE_IPC_ADD_DEP);
		else

			enviar_estado_de_la_dependencia(connectionDC, dep);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_IPC_ADD_NODO_TASK: {
		debug << "Llego mensaje ADD_NODO_TASK\n";
		SocketClass<NodeWorking> sock_msg(conexion);
		NodeWorking* nodeTask = (NodeWorking*) malloc(sizeof(NodeWorking));
		sock_msg >> *nodeTask;
		if (sock_msg.isClosed()) {
                    last_headers[conexion].prox_paso = STATUS_CERRAR_CONEXION;
                    return;
		}
		if (conexion == connectionDC)
			guardar_memoria_de_dc(nodeTask, sizeof(NodeWorking), TYPE_IPC_ADD_NODO_TASK);
		else
			enviar_estado_nodo_tarea(connectionDC, nodeTask);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_IPC_MIGRATION_COMPLETE: {
		debug << "Llego mensaje MIGRATION_COMPLETE\n";
		if (conexion == connectionDC){
			resolver_deseo_de_migracion(conexion);
			last_headers[conexion].prox_paso = STATUS_CERRAR_CONEXION;
		}else{
			enviar_migracion_completa(connectionDC);
            last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		}
		return;
	}
		break;
	case TYPE_IPC_ASSIGN_TASK: {
		debug << "Llego mensaje ASSIGN_TASK\n";
		SocketClass<AssignTask> sock_msg(conexion);
		AssignTask assign;
		sock_msg >> assign;
		if (sock_msg.isClosed()) {
                    last_headers[conexion].prox_paso = STATUS_CERRAR_CONEXION;
                    return;
		}

		enviar_tarea_asignada(conexion, &assign);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_IPC_CANCEL_TASK: {
		debug << "Llego mensaje CANCEL_TASK\n";
		SocketClass<nombre_nodo> sock_msg(conexion);
		nombre_nodo nodo;
		sock_msg >> nodo;
		if (sock_msg.isClosed()) {
                    last_headers[conexion].prox_paso = STATUS_CERRAR_CONEXION;
                    return;
		}

		enviar_cancelar_tarea(nodo);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_IPC_NODO_WORKING: {
		debug << "Working de " << last_headers[conexion].header->id << endl;
		if (connectionDC != 0) {
			SocketClass<insomnio_header> sock_header(connectionDC);
			SocketClass<info_nodo> sock_info(connectionDC);

			nodo_red* nodo = buscar_nodo_en_red(last_headers[conexion].header->id);
			insomnio_header header = { "", TYPE_IPC_NODO_WORKING, 0, 0, 1 };

			sock_header << header;
			sock_info << nodo->info;
		} else {
			info << "llego working del "<< last_headers[conexion].header->id <<"y no tengo dc" << endl;
		}
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	case TYPE_TASK_RESULT: {
		debug << "Llego mensaje TASK RESULT\n";
		SocketClass<TaskResult> sock_msg(conexion);
		TaskResult result;
		sock_msg >> result;
		if (sock_msg.isClosed()) {
                    last_headers[conexion].prox_paso = STATUS_CERRAR_CONEXION;
                    return;
		}

		enviar_al_dc_un_task_result(&result);
                last_headers[conexion].prox_paso = STATUS_LEER_HEADER;
		return;
	}
		break;
	default:
                last_headers[conexion].prox_paso = STATUS_CERRAR_CONEXION;
		error << nodoName << " no se entendio el mensaje payload:" << last_headers[conexion].header->payload << endl;
		return;
	}
	return;
}

void quitar_nodo_segun_conexion_activa(char* nombre) {
    debug << " quitand nodo con nombre " << nombre << endl;
	nodo_red* ant = NULL;
	nodo_red* ptr;
	for (ptr = red_de_nodos()->nodos_salientes; ptr != NULL && strcmp(ptr->info.nombre,nombre); ptr = ptr->next) {
		ant = ptr;
	}
	if (ptr != NULL) {
		if (ant == NULL) {
			red_de_nodos()->nodos_salientes = ptr->next;
		} else {
			ant->next = ptr->next;
		}
//		finalizo_nodo(&ptr->info);
	} else {
		ant = NULL;
		for (ptr = red_de_nodos()->nodos_entrantes; ptr != NULL && strcmp(ptr->info.nombre,nombre); ptr = ptr->next) {
			ant = ptr;
		}
		if (ptr != NULL) {
			if (ant == NULL) {
				red_de_nodos()->nodos_entrantes = ptr->next;
			} else {
				ant->next = ptr->next;
			}
//			finalizo_nodo(&ptr->info);
		}

	}
}


bool viene_por_saliente(char* nombreDelNodoQueLlama, info_nodo nodoDestino) {
	nodo_red* nodo;
	if (nunca_va_a_tener_saliente) {
		return false;
	}
	if (red_de_nodos()->nodos_salientes == NULL && nodoDestino.distancia == 1) {
		sprintf(nodoDestino.nombre_saliente, "%s", nombreDelNodoQueLlama);
		return true;
	}

	for (nodo = red_de_nodos()->nodos_entrantes; nodo != NULL && strcmp(nodo->info.nombre, nombreDelNodoQueLlama); nodo = nodo->next) {
	}
        
	if (nodo != NULL)
		return false;

	//si no lo encontro, lo busco en las salientes
	for (nodo = red_de_nodos()->nodos_salientes; nodo != NULL && strcmp(nodo->info.nombre, nombreDelNodoQueLlama); nodo = nodo->next) {

	}
	return nodo != NULL;
}

#include "messages/teclado-consola.h"
#define CLIENT_INIT_MSG "INSOMNIO CONNECT/1.0\n\n"
#define SERVER_RESPONSE_MSG "INSOMNIO OK\n\n"

bool ConectorCliente::onMessageRecv(PointClass<>* sock) {

    if (last_headers[sock->id()].prox_paso == STATUS_INICIAR_HANDSHAKE) {
        debug << "Conexion " << sock->id() << " iniciando handshake " << endl;
        SocketClass<line> init_hs(sock->id());
        line client_msg;
        init_hs >> client_msg;
        if (init_hs.isClosed()) {
            debug << " Conexion cerrada " << sock->id() << endl;
            last_headers[sock->id()].prox_paso = STATUS_CERRAR_CONEXION;
        } else {
            if (strcmp(client_msg.str, CLIENT_INIT_MSG)) {
                error << "handshake server "<< client_msg.str << str_ip(sock->id()) << ":" << int_puerto(sock->id()) << endl;
                last_headers[sock->id()].prox_paso = STATUS_CERRAR_CONEXION;
            } else {
                debug << "Server handshake ok " << endl;
                line response = {SERVER_RESPONSE_MSG};
                init_hs << response;
                last_headers[sock->id()].prox_paso = STATUS_LEER_HEADER;
                return false;
            }
        }
    }

    if (last_headers[sock->id()].prox_paso == STATUS_LEER_HEADER) {
        debug << "Recibiendo header " << sock->id() << endl;
        SocketClass<insomnio_header> sock_header( sock->id() );
        last_headers[sock->id()].header = new insomnio_header;
        sock_header >> *(last_headers[sock->id()].header);

        if (sock_header.isClosed()) {
            debug << "Conexion cerrada " << sock->id() << endl;
            last_headers[sock->id()].prox_paso = STATUS_CERRAR_CONEXION;
        } else {
            //asignar el proximo paso, seguno el lenth
            if (last_headers[sock->id()].header->length != 0) {
                last_headers[sock->id()].prox_paso = STATUS_TRABAJAR_CONEXION;
                debug << "El header tiene un mensaje encolado, asique espero que llegue " << endl;
                //Si el lenth es != 0, tengo que esperar mas mensajes
                return false;
            } else {
                last_headers[sock->id()].prox_paso = STATUS_LEER_HEADER;
            }
        }
    }

    if ( last_headers[sock->id()].prox_paso != STATUS_CERRAR_CONEXION ) {
        debug << " --- procesando mensaje --- " << endl;
        procesar_mensaje(sock->id());
        debug << " --- fin de proceso de mensaje --- " << endl;
    }

    if (last_headers[sock->id()].prox_paso == STATUS_CERRAR_CONEXION) {
        debug << " Cerrando conexion " << sock->id() << endl;
        if (last_headers[sock->id()].header!=NULL) delete last_headers[sock->id()].header;
        last_headers[sock->id()].header = NULL;
        last_headers[sock->id()].prox_paso = STATUS_INICIAR_HANDSHAKE;

        if (sock->id() == connectionDC) {
            info << "Se desconecto el DC " << connectionDC << endl;
            connectionDC = 0;
            if (red_de_nodos()->mi_nodo->info.dc == true){
            	enviar_dc_caido();
            }
            wait(NULL);
            /*resolver_caida_de_DC(&(red_de_nodos()->mi_nodo->info));*/
        } else {
            nodo_red* nodo_caido = get_nodo_segun_conexion_activa(sock->id());

            if ( nodo_caido != NULL) { // si la conexion que se cerro es de un nodo
                info << " Se cayo el " << nodo_caido->info.nombre << endl;
                if ( !strcmp(nodo_caido->info.nombre,red_de_nodos()->mi_nodo->info.nombre_saliente) ) { // si es mi nodo saliente
                    debug << "Se cayo la conexion con el saliente " << nodo_caido->info.nombre << endl;

                    int nueva_conexion = reconectar_a_otro_saliente(nodo_caido);

                    if (nueva_conexion) {
                        SocketClass<>* sock_saliente = new SocketClass<> (nueva_conexion);
                        listener->add(sock_saliente, this);
                        
                        last_headers[nueva_conexion].prox_paso = STATUS_LEER_HEADER;
                    } else {
                        //no me pude conectar con nadie
                        nunca_va_a_tener_saliente = true;
                    }
                } else {
                    debug << "Se cayo la conexion con un nodo entrante " << nodo_caido->info.nombre << endl;
                }

                //ya esta... el nodo saliente se cayo y encontre otro (o no)
                //asique q saco el ex-nodo saliente de la lista de salientes (o
                //de la lista de entrantes si no encontre saliente)
                quitar_nodo_segun_conexion_activa(nodo_caido->info.nombre);
                finalizo_nodo(&nodo_caido->info);
            }

        }
        return true;
    }

    return false;
}

void ConectorCliente::onMessageSent(PointClass<>* sock) {
}
void ConectorCliente::onMessageError(PointClass<>* sock) {
}

pair<PointClass<>*, SocketEventClass*> ConectorCliente::onNewConnection(SocketGateClass* listener) {
	SocketClass<>* conector = listener->acceptConnection();
        last_headers[conector->id()].prox_paso = STATUS_INICIAR_HANDSHAKE;
//	if (!server_handshake(conector)) {
//		conector->stop();
//		delete conector;
//		conector = NULL;
//	};

	return pair<PointClass<>*, SocketEventClass*> (conector, this);
}

int enviar_msg_ping(char* nodoName, info_nodo info_ping, insomnio_header header) {
	return enviar_ping(nodoName, info_ping, header);
}

int enviar_msg_pong(char* nodoName, info_nodo info_ping, insomnio_header header) {
	return enviar_ping(nodoName, info_ping, header);
}

void init_networkManager(char* configName) {
	SocketBuilderClass* builder = new SocketBuilderClass();
	initADMTask(configName);
	builder->ip(read_config(configName, "mi_ip"));
	builder->puerto(read_config(configName, "mi_puerto"));
	info << "Voy a quedar escuchando en " << read_config(configName, "mi_ip") << ":" << read_config(configName, "mi_puerto") << endl;

	//el orden es importante
	SocketGateClass* tcp = builder->createBind();
	SocketClass<>* udp = builder->createUnbinded();

	Listener* lis = new Listener();

	ConectorCliente* handler = new ConectorCliente(lis, configName);

	lis->add(tcp, handler);
	lis->add(udp, handler);
	lis->loop();

	delete builder;
	delete lis;
}

int enviar_msg_working() {
	insomnio_header header = { "", TYPE_IPC_NODO_WORKING, 0, 0, 0 };
	sprintf(header.id, "%s", nodoName);
	nodo_red* dc = buscar_dc();
	if (dc == NULL) return 0;
	SocketBuilderClass* builder = new SocketBuilderClass();
	builder->ip(dc->info.ip);
	builder->puerto(dc->info.puerto);
        SocketClass<>* udp = builder->startUnconnected();

        if (!udp) return 0;

	SocketClass<insomnio_header> sock(udp->id());
        sock << header;
	debug << "Envie Working " << udp->id() << endl;

	udp->stop();
        delete udp;
	delete builder;
}

int enviar_msg_taskResult(TaskResult* result) {
        debug << "Buscando sock para enviar tarea finalizada" <<  endl;
        nodo_red* nodo = buscar_dc();
    if (nodo == NULL) {
    	return 0;
    }
	SocketBuilderClass* builder = new SocketBuilderClass();
	builder->ip(nodo->info.ip);
	builder->puerto(nodo->info.puerto);
	SocketClass<>* sock = builder->startConnection();

        if (!sock || !client_handshake(sock)) {
            debug << "No pude mandar el task result " << endl;
            return 0;
        }
        
        debug << "Envio la tarea finalizada a " << nodo->info.nombre << " " << nodo->info.conexion << " sock " << sock->id() << endl;
        
        insomnio_header header = { "", TYPE_TASK_RESULT, 0, 0, 1 };
        sprintf(header.id, "%s", nodoName);
        
        SocketClass<insomnio_header> sock_header(sock->id());
        SocketClass<TaskResult> sock_info(sock->id());
        sock_header << header;
        sock_info << *result;

//        debug << " Cerrando conexion " << sock->id() << endl;
        sock->stop();
        
        delete sock;
        delete builder;
        
//        debug << " Listo enviando task result " << endl;
	return 1;
}

int enviar_msg_eleccion(char* destino) {
	insomnio_header header = { "", TYPE_ELECCION, 0, 0, 0 };
	sprintf(header.id, "%s", nodoName);

	return enviar_solo_header_udp(destino, header);
}
int enviar_msg_participacion(char* nombre) {
	insomnio_header header = { "", TYPE_PARTICIPACION, 0, 0, 0 };
	sprintf(header.id, "%s", nodoName);
	info_nodo* nodo = &(red_de_nodos()->mi_nodo->info);
	return enviar_header_y_info(nombre, header, *nodo);
}
int enviar_msg_dcElecto(char* destino, nodo_to_dc nuevo_dc) {
	insomnio_header header = { "", TYPE_DC_ELECTO, 0, 0, 1 };
	sprintf(header.id, "%s", nodoName);
	return enviar_header_y_nodo_dc(destino, header, nuevo_dc);
}

info_red* red_de_nodos() {
	return la_red_;
}

extern pthread_mutex_t mutexLog;

void cambiar_el_DC(nodo_to_dc nodo) {
	debug << "cambiar dc por " << nodo.info.nombre << endl;

	nodo_red* nodoDc = buscar_nodo_en_red(nodo.info.nombre);
	if ((strcmp(nodoDc->info.nombre, red_de_nodos()->mi_nodo->info.nombre) == 0) && (buscar_dc() == NULL)) {
		char mig[2];
		mig[0] = nodo.migracion;
		mig[1] = '\0';
		info << " evento START DC " << nodo.migracion << "  " << mig << " bin " << read_config("./config/daemon.xml", "dc_main.bin") << endl;
		pid_t childpid = fork();
		if (childpid < 0) {
			perror("Fork Failed");
			exit(1);
		} else if (childpid == 0) {
			char* arg[5];
			arg[0] = read_config("./config/daemon.xml", "dc_main.bin");
			arg[1] = red_de_nodos()->mi_nodo->info.ip;
			arg[2] = red_de_nodos()->mi_nodo->info.puerto;
			arg[3] = mig;
			arg[4] = NULL;
			int nerror = execv(arg[0], arg);
			error << " START DC | nunca deberia llegar aca" << nerror << endl;
                        exit(1);
		} else {
                }
		debug << " iniciado DC con pid : " << childpid << endl;
	}
	nodoDc->info.dc = true;
	debug << "Cambio el DC - Viejo:" << (viejoDC == NULL ? "null" : viejoDC->nombre) << "  Nuevo:" << nodo.info.nombre << endl;
}

void en_eleccion_de_nuevo_DC() {
	nodo_red* nodo = buscar_dc();
	if (buscar_dc() != NULL){
		nodo->info.dc = false;
		debug << "seteo el viejo DC " << nodo->info.nombre << endl;
		viejoDC = &nodo->info;
	}else{
		debug << "no habia dc" << endl;
	}
}

int enviar_estado_del_job(int conexion, Job* job) {
	insomnio_header header = { "", TYPE_MIGRACION_JOB, 0, 0, 1 };
	sprintf(header.id, "%s", nodoName);
	debug << "Job enviado por " << conexion << endl;
	sendMessage(conexion, &header, job, sizeof(Job));
}
int enviar_estado_de_la_task(int conexion, Task* task) {
	insomnio_header header = { "", TYPE_MIGRACION_TASK, 0, 0, 1 };
	sprintf(header.id, "%s", nodoName);
	debug << "Task enviado por " << conexion << endl;
	sendMessage(conexion, &header, task, sizeof(Task));
}
int enviar_estado_de_la_dependencia(int conexion, DependencyNet* dep) {
	insomnio_header header = { "", TYPE_MIGRACION_DEP, 0, 0, 1 };
	sprintf(header.id, "%s", nodoName);
	debug << "Dep enviado por " << conexion << endl;
	sendMessage(conexion, &header, dep, sizeof(DependencyNet));
}
int enviar_estado_nodo_tarea(int conexion, NodeWorking* nodoTask) {
	insomnio_header header = { "", TYPE_MIGRACION_NODO, 0, 0, 1 };
	sprintf(header.id, "%s", nodoName);
	debug << "Nodo Tarea enviado por " << conexion << endl;
	sendMessage(conexion, &header, nodoTask, sizeof(NodeWorking));
}
int enviar_migracion_completa(int conexion) {
	nodo_red* nodo = buscar_dc();
	insomnio_header header = { "", TYPE_MIGRACION_COMPLETE, 0, 0, 0 };
	sprintf(header.id, "%s", nodoName);
	debug << "Migracion completa" << endl;
	enviar_solo_header(conexion, header);
	return 0;
}
void event_startDC(char migration) {
	nodo_to_dc nodo = { red_de_nodos()->mi_nodo->info, migration };
	cambiar_el_DC(nodo);
}
void nuevo_nodo(info_nodo* nodo) {
	debug << "Nuevo nodo " << endl;
	if (red_de_nodos()->mi_nodo->info.dc) {
            debug << "Soy DC " << endl;
            insomnio_header header = { "", TYPE_IPC_NEW_NODO, 0, 0, 1 };
            sprintf(header.id, "%s", nodoName);
            if (connectionDC > 0) {
                    debug << "Nuevo nodo " << nodo->nombre << " se lo digo a DC" << endl;
                    enviar_header_e_info_nodo(connectionDC, header, nodo);
            }
	}
}
void finalizo_nodo(info_nodo* nodo) {
	debug << "Finalizo nodo " << nodo->nombre << endl;
	if (red_de_nodos()->mi_nodo->info.dc) {
            debug << "Soy DC " << endl;
            insomnio_header header = { "", TYPE_IPC_ENDED_NODO, 0, 0, 1 };
            sprintf(header.id, "%s", nodoName);
            if (connectionDC > 0) {
                    debug << "Finalizo nodo " << nodo->nombre << " se lo digo al DC" << endl;
                    enviar_header_e_info_nodo(connectionDC, header, nodo);
            }
	}
	if ((buscar_dc() == NULL) && (connectionDC == NULL)){
		resolver_caida_de_DC(NULL, '0');
	}
}
