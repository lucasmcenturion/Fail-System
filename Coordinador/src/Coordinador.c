#include "sockets.h"

char *ALGORITMO_DISTRIBUCION;
int PUERTO, CANT_ENTRADAS, TAMANIO_ENTRADA, RETARDO;

void obtenerValoresArchivoConfiguracion() {
	t_config* arch = config_create("/home/utnso/workspace/tp-2018-1c-Fail-system/Coordinador/coordinadorCFG.txt");
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
				"PUERTO=%d\n"
				"ALGORITMO_DISTRIBUCION=%s\n"
				"CANT_ENTRADAS=%d\n"
				"TAMANIO_ENTRADA=%d\n"
				"RETARDO=%d\n",
				PUERTO,
				ALGORITMO_DISTRIBUCION,
				CANT_ENTRADAS,
				TAMANIO_ENTRADA,
				RETARDO
				);
	fflush(stdout);
}

int main(void) {
	obtenerValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
	return EXIT_SUCCESS;
}
