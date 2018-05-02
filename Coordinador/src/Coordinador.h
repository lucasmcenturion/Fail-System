#ifndef DATANODE_H_
#define DATANODE_H_
/*#include <general/archivoConfig.h>
#include <general/serializacion.h>
#include <general/logs.h>
#include <general/sockets.h> */
#include <sys/mman.h> //librería mmap
#include <commons/config.h>
#include <commons/log.h>
#include <stdio.h>
#endif

/* Funciones */

void obtenerValoresArchivoConfiguracion();
void imprimirArchivoConfiguracion();
void accion(void* socket);


/* Variables Globales */
char *IP, *ALGORITMO_DISTRIBUCION;
int PUERTO, CANT_ENTRADAS, TAMANIO_ENTRADA, RETARDO;
// t_list* listaHilos;
bool end;

