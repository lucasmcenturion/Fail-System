#include "sockets.h"
#include "Coordinador.h"

/* char *IP, *ALGORITMO_DISTRIBUCION;
int PUERTO_COORDINADOR, CANT_ENTRADAS, TAMANIO_ENTRADA, RETARDO;
t_list* listaHilos;
bool end;  */  //Las dejo como variables globales en el .h

t_list* listaHilos; //No se por qué me tira error en el Coordinador.h

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

/* int comprobarValoresBienSeteados() {

	int retorno = 1;

	if (!PUERTO) {
		logearError(
				"Archivo config de Coordinador: el puerto del COORDINADOR está mal configurado.");
		retorno = 0;
	}


	return retorno;
} */ //Necesito la general

void accion(void* socket) {
	int socketFD = *(int*) socket;
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteServidor(socketFD, COORDINADOR, &paquete) > 0) {
	//SWITCH
	}
}





/* int main(void) {

	obtenerValoresArchivoConfiguracion();

	imprimirArchivoConfiguracion();

	ServidorConcurrente(IP, PUERTO, COORDINADOR, &listaHilos, &end, accion);

	return EXIT_SUCCESS;
} */
