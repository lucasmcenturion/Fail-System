#include "sockets.h"

char *IP_COORDINADOR, *IP_PLANIFICADOR;
int PUERTO_COORDINADOR, PUERTO_PLANIFICADOR;

void obtenerValoresArchivoConfiguracion() {
	t_config* arch = config_create("/home/utnso/workspace/tp-2018-1c-Fail-system/ESI/esiCFG.txt");
	IP_COORDINADOR = string_duplicate(config_get_string_value(arch, "IP_COORDINADOR"));
	PUERTO_COORDINADOR = config_get_int_value(arch, "PUERTO_COORDINADOR");
	IP_PLANIFICADOR = string_duplicate(config_get_string_value(arch, "IP_PLANIFICADOR"));
	PUERTO_PLANIFICADOR = config_get_int_value(arch, "PUERTO_PLANIFICADOR");
	config_destroy(arch);
}

void imprimirArchivoConfiguracion(){
	printf(
				"Configuración:\n"
				"IP_COORDINADOR=%s\n"
				"PUERTO_COORDINADOR=%d\n"
				"IP_PLANIFICADOR=%s\n"
				"PUERTO_PLANIFICADOR=%d\n",
				IP_COORDINADOR,
				PUERTO_COORDINADOR,
				IP_PLANIFICADOR,
				PUERTO_PLANIFICADOR
				);
	fflush(stdout);
}

int main(int argc, char* argv[]) {
	obtenerValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
	char* programaAEjecutar = argv[1];
	printf("el programa a ejecutar es %s", programaAEjecutar);
	fflush(stdout);
	ConectarAServidor(PUERTO_PLANIFICADOR, IP_PLANIFICADOR, PLANIFICADOR, ESI, RecibirHandshake);
	ConectarAServidor(PUERTO_COORDINADOR, IP_COORDINADOR, COORDINADOR, ESI, RecibirHandshake);
	return EXIT_SUCCESS;
}
