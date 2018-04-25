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

void PausarContinuar(){

}

void Bloquear(){

}

void Desbloquear(){

}

void Listar(){

}

void Kill(){

}

void Status(){

}

void Deadlock(){

}
void consola() {
	char * linea;
	while (true) {
		linea = readline(">> ");
		if (linea)
			add_history(linea);
		if (!strcmp(linea, "pausar/continuar")) {
			printf("Pausó/Continuó.\n");
			PausarContinuar();
		}
		else if (!strncmp(linea, "bloquear ", 9)) {
			char **array_input = string_split(linea, " ");
			printf("Se bloqueó el proceso ESI de id %s, en la cola del recurso %s.\n", array_input[2], array_input[1]);
			Bloquear();
			string_iterate_lines(array_input,free);
			free(array_input);
		}
		else if (!strncmp(linea, "desbloquear ", 12)) {
			char **array_input = string_split(linea, " ");
			printf("Se desbloqueó el primer proceso ESI en la cola del recurso %s.\n", array_input[1]);
			Desbloquear();
			string_iterate_lines(array_input,free);
			free(array_input);
		}
		else if (!strncmp(linea, "listar ", 7)) {
			char **array_input = string_split(linea, " ");
			printf("Se listan los procesos bloqueados esperando el recurso %s.\n", array_input[1]);
			Listar();
			string_iterate_lines(array_input,free);
		free(array_input);
		}

		else if (!strncmp(linea, "kill ", 5)) {
			char **array_input = string_split(linea, " ");
			printf("Se finaliza el proceso de id %s.\n", array_input[1]);
			Kill();
			string_iterate_lines(array_input,free);
			free(array_input);
		}
		else if (!strncmp(linea, "status ", 7)) {
			char **array_input = string_split(linea, " ");
			printf("Se muestra el estado de la clave %s.\n", array_input[1]);
			Status();
			string_iterate_lines(array_input,free);
			free(array_input);
		}
		else if (!strcmp(linea, "deadlock")) {
			printf("Se muestra deadlocks del sistema.\n");
			Deadlock();
		}
		else if (!strcmp(linea, "exit")) {
			printf("Salió\n");
			free(linea);
			break;
		} else
			printf("No se conoce el comando\n");
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
