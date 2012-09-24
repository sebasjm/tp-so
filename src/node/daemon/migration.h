/*
 * migration.h
 *
 *  Created on: 16/11/2010
 *      Author: pmarchesi
 */

#ifndef MIGRATION_H_
#define MIGRATION_H_

#include <messages/daemon-daemon.h>

void resolver_caida_de_DC(info_nodo* caido, char migracion);
void resolver_msg_eleccion(char* conexion);
void resolver_msg_participacion(info_nodo participa);
void resolver_msg_dcElecto(nodo_to_dc nuevoDC);
void init_migration(char* configName);

#endif /* MIGRATION_H_ */
