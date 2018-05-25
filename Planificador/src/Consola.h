/*
 * Consola.h
 *
 *  Created on: 5 may. 2018
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include "sockets.h"

extern bool planificacion_detenida;

void PausarContinuar();
void Bloquear(char* clave, char* id);
void Desbloquear(char* clave);
void Listar(char* recurso);
void Kill();
void Status();
void Deadlock();
void consola();

#endif /* CONSOLA_H_ */
