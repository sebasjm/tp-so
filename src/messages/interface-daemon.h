#ifndef __interface_daemon_h
#define __interface_daemon_h

/**
 * En este archivo van todas las estructuras de mensajes
 * que se utilicen para comunicar la interfaz con el daemon
 */
typedef struct un_mensaje {
    char str[12];
} mensaje;

#endif
