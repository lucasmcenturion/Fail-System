#include "sockets.h"

char *IP_COORDINADOR, *ALGORITMO_REEMPLAZO, *PUNTO_MONTAJE, *NOMBRE_INSTANCIA;
int PUERTO_COORDINADOR, INTERVALO_DUMP;

void obtenerValoresArchivoConfiguracion() {
	t_config* arch = config_create("/home/utnso/workspace/tp-2018-1c-Fail-system/Instancia/instanciaCFG.txt");
	IP_COORDINADOR = string_duplicate(config_get_string_value(arch, "IP_COORDINADOR"));
	PUERTO_COORDINADOR = config_get_int_value(arch, "PUERTO_COORDINADOR");
	ALGORITMO_REEMPLAZO = string_duplicate(config_get_string_value(arch, "ALGORITMO_REEMPLAZO"));
	PUNTO_MONTAJE = string_duplicate(config_get_string_value(arch, "PUNTO_MONTAJE"));
	NOMBRE_INSTANCIA = string_duplicate(config_get_string_value(arch, "NOMBRE_INSTANCIA"));
	INTERVALO_DUMP = config_get_int_value(arch, "INTERVALO_DUMP");
	config_destroy(arch);
}

void imprimirArchivoConfiguracion(){
	printf(
				"Configuración:\n"
				"IP_COORDINADOR=%s\n"
				"PUERTO_COORDINADOR=%d\n"
				"ALGORITMO_REEMPLAZO=%s\n"
				"PUNTO_MONTAJE=%s\n"
				"NOMBRE_INSTANCIA=%s\n"
				"INTERVALO_DUMP=%d\n",
				IP_COORDINADOR,
				PUERTO_COORDINADOR,
				ALGORITMO_REEMPLAZO,
				PUNTO_MONTAJE,
				NOMBRE_INSTANCIA,
				INTERVALO_DUMP
				);
	fflush(stdout);
}

int main(void) {
	obtenerValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
	return EXIT_SUCCESS;
}
