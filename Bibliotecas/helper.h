#ifndef HELPER_
#define HELPER_
#define LAMBDA(c_) ({ c_ _;}) //Para funciones lambda

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <commons/temporal.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <commons/txt.h>
#include <commons/bitarray.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#include <ctype.h>
#include <semaphore.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>

#define true 1
#define false 0
#define NULL ((void *) 0)

void escribir_log(char* nombre_log,char* proceso,char* mensaje,char* tipo);
char* integer_to_string(char*string,int x);
size_t getFileSize(const char* filename);
void planificar(void);
void ejecutarEsi(void);
void ChequearPlanificacionYSeguirEjecutando(void);
void PasarESIMuertoAColaTerminados(char*);
void parsear(void);
void muerteEsi(void);


typedef struct {
	int socket;
	int inicialKE;
	int finalKE;
	char* nombre;
	bool activo;
	t_list *claves;
}__attribute__((packed)) t_IdInstancia;

typedef struct {
	char* clave;
	int index;
	int entradasOcupadas;
	int tamanio;
	bool atomico;
	bool activo;
}__attribute__((packed)) t_Entrada;

typedef struct {
	int socket;
	char* id;
	int rafagasRealesEjecutadas;
	float rafagasEstimadas;
}__attribute__((packed)) procesoEsi;

typedef struct {
	procesoEsi* esi;
	char* clave;
}__attribute__((packed)) esiBloqueado;

typedef struct {
	char* clave;
	char* idEsi;
}__attribute__((packed)) clavexEsi;

typedef struct {
	char* id;
	int socket;
	t_list* claves;
}__attribute__((packed)) t_esiCoordinador;
#endif /* HELPER_*/
