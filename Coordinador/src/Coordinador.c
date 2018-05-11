#include "sockets.h"
#define logearError(msg) {log_error(vg_logger,msg);}

/* Variables Globales */
char *IP, *ALGORITMO_DISTRIBUCION;
int PUERTO, CANT_ENTRADAS, TAMANIO_ENTRADA, RETARDO,socketCoordinador;
t_list* listaHilos;
bool end;
t_log * vg_logger = NULL;
t_list* instancias;
t_list* esis;
t_list* claves_globales;
pthread_mutex_t mutex_instancias;
pthread_mutex_t mutex_esis;
pthread_mutex_t mutex_claves;

void obtenerValoresArchivoConfiguracion() {
	t_config* arch = config_create("/home/utnso/workspace/tp-2018-1c-Fail-system/Coordinador/coordinador.cfg");
	IP = string_duplicate(config_get_string_value(arch, "IP"));
	PUERTO = config_get_int_value(arch, "PUERTO");
	ALGORITMO_DISTRIBUCION = string_duplicate(config_get_string_value(arch, "ALGORITMO_DISTRIBUCION"));
	CANT_ENTRADAS = config_get_int_value(arch, "CANT_ENTRADAS");
	TAMANIO_ENTRADA = config_get_int_value(arch, "TAMANIO_ENTRADA");
	RETARDO = config_get_int_value(arch, "RETARDO");
	config_destroy(arch);
}

void imprimirArchivoConfiguracion(){

	printf(
				"Configuración:\n"
				"IP=%s\n"
				"PUERTO=%d\n"
				"ALGORITMO_DISTRIBUCION=%s\n"
				"CANT_ENTRADAS=%d\n"
				"TAMANIO_ENTRADA=%d\n"
				"RETARDO=%d\n",
				IP,
				PUERTO,
				ALGORITMO_DISTRIBUCION,
				CANT_ENTRADAS,
				TAMANIO_ENTRADA,
				RETARDO
				);
	fflush(stdout);
}

int comprobarValoresBienSeteados() {

	int retorno = 1;

	if (!PUERTO) {
		logearError(
				"Archivo config de Coordinador: el puerto del COORDINADOR está mal configurado.");
		retorno = 0;
	}


	return retorno;
}
int getProximo(){
	if(list_size(instancias)==0)
		return 0;
	t_IdInstancia *aux;

	if(list_all_satisfy(instancias,LAMBDA(bool _(t_IdInstancia * elemento) { return elemento->activo==true;}))){
		list_iterate(instancias, LAMBDA(void _(t_IdInstancia * elemento) {  elemento->activo=false;}));
		aux= list_get(instancias,0);
		aux->activo=true;
		list_replace(instancias,0,aux);
		return aux->socket;
	}
	int i=-1;
	bool proximo (t_IdInstancia *elemento){
		i++;
		if(!elemento->activo)
			return true;
	}
	list_find(instancias, proximo);
	aux= list_get(instancias, i);
	aux->activo=true;
	list_replace(instancias,i,aux);
	return aux->socket;
}

void sacar_instancia(int socket) {
	int tiene_socket(t_IdInstancia *instancia) {
		if (instancia->socket == socket)
			return instancia->socket != socket;
	}
	instancias = list_filter(instancias, (void*) tiene_socket);
}


void accion(void* socket) {
	int socketFD = *(int*) socket;
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteServidorCoordinador(socketFD, COORDINADOR, &paquete) > 0) {
	//SWITCH
		if (!strcmp(paquete.header.emisor, INSTANCIA)) {
			switch(paquete.header.tipoMensaje){
				case ESHANDSHAKE:{
					EnviarDatosTipo(socketFD,COORDINADOR,1,0,SOLICITUDNOMBRE);
					int tamanioDatosEntradas=sizeof(int)*2;
					void *datosEntradas=malloc(tamanioDatosEntradas);
					*((int*)datosEntradas)=TAMANIO_ENTRADA;
					datosEntradas+=sizeof(int);
					*((int*)datosEntradas)=CANT_ENTRADAS;
					datosEntradas+=sizeof(int);
					datosEntradas-=tamanioDatosEntradas;
					EnviarDatosTipo(socketFD,COORDINADOR,datosEntradas,tamanioDatosEntradas,GETENTRADAS);
					free(datosEntradas);
				}
				break;
				case IDENTIFICACIONINSTANCIA: {
					char *nombreInstancia = malloc(paquete.header.tamPayload);
					strcpy(nombreInstancia,(char*)paquete.Payload);
					t_IdInstancia *instancia= malloc(sizeof(t_IdInstancia));
					instancia->socket=socketFD;
					instancia->nombre=malloc(strlen(nombreInstancia)+1);
					instancia->activo = false;
					instancia->claves = list_create();
					strcpy(instancia->nombre,nombreInstancia);
					pthread_mutex_lock(&mutex_instancias);
					list_add(instancias,instancia);
					pthread_mutex_unlock(&mutex_instancias);
				}
				break;
				case SETOK:
				{
					int tiene_socket(t_IdInstancia *e) {
						if (e->socket == socketFD)
							return e->socket != socketFD;
					}
					pthread_mutex_lock(&mutex_instancias);
					list_add(((t_IdInstancia*)list_find(instancias, tiene_socket)),(char*)paquete.Payload);
					pthread_mutex_unlock(&mutex_instancias);
					pthread_mutex_lock(&mutex_claves);
					list_add(claves_globales,(char*)paquete.Payload);
					pthread_mutex_unlock(&mutex_claves);
				}
				break;
			}
		}
		if(!strcmp(paquete.header.emisor, ESI)){
			switch(paquete.header.tipoMensaje){
				case SETCOORD:{
					if(!strcmp(ALGORITMO_DISTRIBUCION,"EL")){
						pthread_mutex_lock(&mutex_instancias);
						int socketSiguiente = getProximo();
						if(socketSiguiente!=0){
							printf("%s\n",socketSiguiente);
							fflush(stdout);
						}else{
							//error, no hay instancias conectadas al sistema
						}
						pthread_mutex_unlock(&mutex_instancias);
					}
				}
				break;
				case ESHANDSHAKE:{
					t_esiCoordinador*nuevo =malloc(sizeof(t_esiCoordinador));
					nuevo->id=malloc(paquete.header.tamPayload);
					strcpy(nuevo->id,paquete.Payload);
					nuevo->socket = socketFD;
					pthread_mutex_lock(&mutex_esis);
					list_add(esis,nuevo);
					pthread_mutex_unlock(&mutex_esis);
				}
				break;
				case GETCOORD: {
					bool foo(char*e){
						return !strcmp(e,(char*)paquete.Payload);
					}
					bool verificarClave(t_IdInstancia *e){
						return list_any_satisfy(e->claves,LAMBDA(int _(char *clave) {  return !strcmp(clave,(char*)paquete.Payload);}));
					}
					char*id=malloc(10);
					strcpy(id,((t_esiCoordinador*)list_find(esis,LAMBDA(int _(t_esiCoordinador *elemento) {  return elemento->socket ==socketFD;})))->id);
					id=realloc(id,strlen(id)+1);
					pthread_mutex_lock(&mutex_claves);
					if(list_any_satisfy(claves_globales, foo)){
						if(!list_any_satisfy(instancias, verificarClave)){
							//clave eviste en el sistema, pero la instancia esta caida
							printf("Error clave innacesible");
							fflush(stdout);
							EnviarDatosTipo(socketCoordinador,COORDINADOR,id,strlen(id)+1,ABORTAR);
						}else{
							EnviarDatosTipo(socketCoordinador,COORDINADOR,id,strlen(id)+1,GETPLANI);
						}

					}else{
						//clave no existe en el sistema
						EnviarDatostipo(socketCoordinador,COORDINADOR,id,strlen(id)+1,ABORTAR);
					}
					pthread_mutex_unlock(&mutex_claves);
				}
			}
		}
		if(!strcmp(paquete.header.emisor, COORDINADOR)){
			switch(paquete.header.tipoMensaje){
				case ESHANDSHAKE: {
					socketCoordinador = socketFD;
				}
				break;
			}
		}
		if (paquete.Payload != NULL)
			free(paquete.Payload);
	}
	close(socketFD);
	pthread_mutex_lock(&mutex_instancias);
	sacar_instancia(socketFD);
	pthread_mutex_unlock(&mutex_instancias);
}






int main(void) {

	obtenerValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
	pthread_mutex_init(&mutex_instancias,NULL);
	pthread_mutex_init(&mutex_esis,NULL);
	pthread_mutex_init(&mutex_claves,NULL);
	esis=list_create();
	claves_globales=list_create();
	instancias=list_create();
	ServidorConcurrente(IP, PUERTO, COORDINADOR, &listaHilos, &end, accion);

	return EXIT_SUCCESS;
}
