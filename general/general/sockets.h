#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>     //memset
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>      //perror
#include <arpa/inet.h>  //INADDR_ANY
#include <unistd.h>     //close  usleep
#include <netdb.h> 		//gethostbyname
#include <netinet/in.h>
#include <fcntl.h> //fcntl

#define FAIL -1
#define manejaError(msj) {perror(msj); abort();}

// Servidor
int  newSocket();
struct sockaddr_in asociarSocket(int sockfd, int puerto);
void escucharSocket(int sockfd, int conexionesEntrantesPermitidas);
int  aceptarConexionSocket(int sockfd);

// Cliente
int conectarSocket(int sockfd, const char * ipDestino, int puerto);
int conectarseAServer(char* ip, int port);

// Enviar y Recibir
int enviarPorSocket(int fdCliente, const void * mensaje, int tamanio);
int recibirPorSocket(int fdCliente, void * buffer, int tamanio) ; // retorna -1 si fallo, 0 si se desconecto o los bytes recibidos

// Cerrar File Descriptor
void cerrarSocket(int sockfd);

// Select
void seleccionarSocket(int maxNumDeFD,
					   fd_set * fdListoLectura, fd_set * fdListoEscritura, fd_set * fdListoEjecucion,
					   int* segundos, int* miliSegundos);

#endif /* SOCKETS_H_ */
