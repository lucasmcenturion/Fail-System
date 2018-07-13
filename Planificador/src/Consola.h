#ifndef CONSOLA_H_
#define CONSOLA_H_

#include "sockets.h"

extern bool planificacion_detenida;
extern t_list *LISTOS, *EJECUCION, *BLOQUEADOS, *clavesBloqueadas;
extern sem_t semPlanificacionDetenida;
extern t_log*logger;
extern int flag;
extern int tiempoActual;

void PausarContinuar();
void Bloquear(char* clave, char* id);
void Desbloquear(char* clave, bool flagPrint);
void Listar(char* recurso);
void Kill(char* id);
void Status(char* clave);
void Deadlock();
void consola();

#endif
