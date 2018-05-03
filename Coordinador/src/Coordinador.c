#include "sockets.h"
#define logearError(msg) {log_error(vg_logger,msg);}

/* Variables Globales */
char *IP, *ALGORITMO_DISTRIBUCION;
int PUERTO, CANT_ENTRADAS, TAMANIO_ENTRADA, RETARDO;
t_list* listaHilos;
bool end;
t_log * vg_logger = NULL;
t_list* instancias;
pthread_mutex_t mutex_instancias;


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
bool inactivo (t_IdInstancia * elemento){
		return elemento->activo;
}
int getProximo(){
	if(list_size(instancias)==0)
		return 0;
	t_IdInstancia *aux;
	if(!list_find(instancias, LAMBDA(bool _(t_IdInstancia * elemento) { return elemento->activo;}))){
		list_iterate(instancias, LAMBDA(void _(t_IdInstancia * elemento) {  elemento->activo=false;}));
		t_IdInstancia* nuevo = list_get(instancias,0);
		nuevo->activo=true;
		list_replace(instancias,0,nuevo);
		return ((t_IdInstancia*)list_get(instancias,0))->socket;
	}
	t_IdInstancia * replace= list_find(instancias, LAMBDA(bool _(t_IdInstancia * elemento) { return elemento->activo;}));
	replace->activo=true;
	return replace->socket;

}
void accion(void* socket) {
	int socketFD = *(int*) socket;
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteServidor(socketFD, COORDINADOR, &paquete) > 0) {
	//SWITCH
		if (!strcmp(paquete.header.emisor, INSTANCIA)) {
			switch(paquete.header.tipoMensaje){
				case ESHANDSHAKE:{
					void *datosNombre=malloc(sizeof(int));
					EnviarDatosTipo(socketFD,COORDINADOR,datos,0,SOLICITUDNOMBRE);
					free(datosNombre);
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
					strcpy(instancia->nombre,nombreInstancia);
					pthread_mutex_lock(&mutex_instancias);
					list_add(instancias,instancia);
					pthread_mutex_unlock(&mutex_instancias);
					getProximo();
				}
				break;
			}
		}
		if(!strcmp(paquete.header.emisor, ESI)){
			switch(paquete.header.tipoMensaje){
				case NUEVAOPERACION:{
					//aca se recibe la operacion
					//recibir linea del ESI, separarlo por espacios, verificar el primer elemento
					//si es SET,GET o STORE
					if(!strcmp(ALGORITMO_DISTRIBUCION,"EL")){
						int socketSiguiente = getProximo();
						if(socketSiguiente!=0){

						}else{
							//error, no hay instancias conectadas al sistema
						}
					}
				}
				break;
			}
		}
		if (paquete.Payload != NULL)
			free(paquete.Payload);
	}
	close(socketFD);
}






int main(void) {

	obtenerValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
	pthread_mutex_init(&mutex_instancias,NULL);
	instancias=list_create();
	ServidorConcurrente(IP, PUERTO, COORDINADOR, &listaHilos, &end, accion);

	return EXIT_SUCCESS;
}
