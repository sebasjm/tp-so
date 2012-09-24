#ifndef __commands_h
#define __commands_h

#include <ipc/tecladoConsola.h>

typedef void (*command_func)(TecladoConsolaClass sock, char* nombre_nodo);

struct commands_info {
    const char* name;
    const command_func resolver;
};

void commands_start   (TecladoConsolaClass sock, char* nombre_nodo);
void commands_stop    (TecladoConsolaClass sock, char* nombre_nodo);
void commands_display (TecladoConsolaClass sock, char* nombre_nodo);
void commands_help    (TecladoConsolaClass sock, char* nombre_nodo);
void commands_list    (TecladoConsolaClass sock, char* nombre_nodo);
void commands_exit    (TecladoConsolaClass sock, char* nombre_nodo);
void commands_refresh (TecladoConsolaClass sock, char* nombre_nodo);
void commands_running (TecladoConsolaClass sock, char* nombre_nodo);

#endif
