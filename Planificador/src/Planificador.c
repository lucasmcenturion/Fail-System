#include "sockets.h"

char *ALGORITMO_PLANIFICACION, *IP_COORDINADOR;
int PUERTO, ESTIMACION_INICIAL, PUERTO_COORDINADOR;
char **CLAVES_BLOQUEADAS;

void obtenerValoresArchivoConfiguracion() {
	t_config* arch = config_create("/home/utnso/workspace/tp-2018-1c-Fail-system/Planificador/planificadorCFG.txt");
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
				"PUERTO=%d\n"
				"ALGORITMO_PLANIFICACION=%s\n"
				"ESTIMACION_INICIAL=%d\n"
				"IP_COORDINADOR=%s\n"
				"PUERTO_COORDINADOR=%d\n"
				"CLAVES_BLOQUEADAS=\n",
				PUERTO,
				ALGORITMO_PLANIFICACION,
				ESTIMACION_INICIAL,
				IP_COORDINADOR,
				PUERTO_COORDINADOR
				);
	string_iterate_lines(CLAVES_BLOQUEADAS, LAMBDA(void _(char* item1) { printf("\t\t   %s\n", item1);}));
	fflush(stdout);
}

int main(void) {
	obtenerValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
	return EXIT_SUCCESS;
}
