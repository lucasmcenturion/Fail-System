#include "sockets.h"

char *IP, *ALGORITMO_PLANIFICACION, *IP_COORDINADOR;
int PUERTO, ESTIMACION_INICIAL, PUERTO_COORDINADOR;
char **CLAVES_BLOQUEADAS;

int socketCoordinador;

t_list* listaHilos;
bool end;

void obtenerValoresArchivoConfiguracion() {
	t_config* arch = config_create("/home/utnso/workspace/tp-2018-1c-Fail-system/Planificador/planificadorCFG.txt");
	IP = string_duplicate(config_get_string_value(arch, "IP"));
	PUERTO = config_get_int_value(arch, "PUERTO");
	ALGORITMO_PLANIFICACION = string_duplicate(config_get_string_value(arch, "ALGORITMO_PLANIFICACION"));
	ESTIMACION_INICIAL = config_get_int_value(arch, "ESTIMACION_INICIAL");
	IP_COORDINADOR = string_duplicate(config_get_string_value(arch, "IP_COORDINADOR"));
	PUERTO_COORDINADOR = config_get_int_value(arch, "PUERTO_COORDINADOR");
	CLAVES_BLOQUEADAS = config_get_array_value(arch, "CLAVES_BLOQUEADAS");
	config_destroy(arch);
}

void imprimirArchivoConfiguracion(){
	printf(
				"Configuración:\n"
				"IP=%s\n"
				"PUERTO=%d\n"
				"ALGORITMO_PLANIFICACION=%s\n"
				"ESTIMACION_INICIAL=%d\n"
				"IP_COORDINADOR=%s\n"
				"PUERTO_COORDINADOR=%d\n"
				"CLAVES_BLOQUEADAS=\n",
				IP,
				PUERTO,
				ALGORITMO_PLANIFICACION,
				ESTIMACION_INICIAL,
				IP_COORDINADOR,
				PUERTO_COORDINADOR
				);
	string_iterate_lines(CLAVES_BLOQUEADAS, LAMBDA(void _(char* item1) { printf("\t\t   %s\n", item1);}));
	fflush(stdout);
}


void consola() {
	char * linea;
	while (true) {
		linea = readline(">> ");
		if (linea)
			add_history(linea);
		if (!strcmp(linea, "Pausar/Continuar")) {
			printf("Pausó/Continuó\n");
		}
		else if (!strcmp(linea, "exit")) {
			printf("salio\n");
			free(linea);
			break;
		} else
			printf("no se conoce el comando\n");
		free(linea);
	}
}


void accion(void* socket) {
	int socketFD = *(int*) socket;
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteServidor(socketFD, COORDINADOR, &paquete) > 0) {
	//SWITCH
	}
}

int main(void) {
	obtenerValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
	socketCoordinador = ConectarAServidor(PUERTO_COORDINADOR, IP_COORDINADOR, COORDINADOR, PLANIFICADOR, RecibirHandshake);
	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void*) consola, NULL);
	ServidorConcurrente(IP,PUERTO, PLANIFICADOR, &listaHilos, &end, accion);
	pthread_join(hiloConsola, NULL);
	return EXIT_SUCCESS;
}
