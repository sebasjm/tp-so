#ifndef __daemon_daemon_h
#define __daemon_daemon_h

typedef struct {
    char id[15];
} nombre_nodo;

typedef struct {
    char id[15];
    int payload;
    int ttl;
    int hops;
    long length;
} insomnio_header;

typedef struct {
    char ip[16];
    char puerto[6];
    char nombre[15];
    char nombre_saliente[15];
    int dc;
    int distancia;
    unsigned long memoria;
    unsigned long disco;
    int conexion;
    int tiempo_sin_pong;
} info_nodo;
typedef struct {
	 info_nodo info;
	 char migracion;
}nodo_to_dc;

typedef struct {
    nombre_nodo nombre;
    info_nodo info;
} msg_red;

typedef struct st_nodo_red {
    info_nodo info;
    struct st_nodo_red* next;
} nodo_red;

typedef struct {
    nodo_red* nodos_entrantes;
    nodo_red* nodos_salientes;
    nodo_red* mi_nodo;
} info_red;

typedef struct {
    int field1;
    char field2;
    long field3;
    double field4;
    float field5;
    void* field6;
    long field7;
} estructura_complicada;
#endif
