#ifndef SOCKETS_H_
#define SOCKETS_H_

#include "helper.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10 // Cuántas conexiones pendientes se mantienen en cola del server
#define TAMANIOHEADER sizeof(Header)
#define STRHANDSHAKE "10"
//Emisores:
#define ESI "ESI         "
#define INSTANCIA "Instancia   "
#define COORDINADOR "Coordinador "
#define PLANIFICADOR "Planificador"

typedef enum { ESHANDSHAKE, ESDATOS, ESSTRING, ESARCHIVO, ESINT, ESERROR, IDENTIFICACIONINSTANCIA,SOLICITUDNOMBRE, GET, SET,
				GETENTRADAS, NUEVAOPERACION} tipo;

typedef struct {
	tipo tipoMensaje;
	uint32_t tamPayload;
	char emisor[13];
}__attribute__((packed)) Header;

typedef struct {
	Header header;
	void* Payload;
}__attribute__((packed)) Paquete;

typedef struct {
	pthread_t hilo;
	int socket;
} structHilo;

typedef struct {
	pid_t pid;
	int socket;
} structProceso;

void Servidor(char* ip, int puerto, char nombre[13],
		void (*accion)(Paquete* paquete, int socketFD),
		int (*RecibirPaquete)(int socketFD, char receptor[13], Paquete* paquete));
void ServidorConcurrente(char* ip, int puerto, char nombre[13], t_list** listaDeHilos,
		bool* terminar, void (*accionHilo)(void* socketFD));
void ServidorConcurrenteForks(char* ip, int puerto, char nombre[13], t_list** listaDeProcesos,
		bool* terminar, void (*accionPadre)(void* socketFD), void (*accionHijo)(void* socketFD));
int StartServidor(char* MyIP, int MyPort);
int ConectarAServidor(int puertoAConectar, char* ipAConectar, char servidor[13], char cliente[13],
		void RecibirElHandshake(int socketFD, char emisor[13])); //Sobrecarga para Kernel

bool EnviarDatos(int socketFD, char emisor[13], void* datos, int tamDatos);
bool EnviarDatosTipo(int socketFD, char emisor[13], void* datos, int tamDatos, tipo tipoMensaje);
bool EnviarMensaje(int socketFD, char* msg, char emisor[13]);
bool EnviarPaquete(int socketCliente, Paquete* paquete);
void RecibirHandshake(int socketFD, char emisor[13]);
int RecibirPaqueteServidor(int socketFD, char receptor[13], Paquete* paquete); //Responde al recibir un Handshake
int RecibirPaqueteCliente(int socketFD, char receptor[13], Paquete* paquete); //No responde los Handshakes

#endif //SOCKETS_H_
