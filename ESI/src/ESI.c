#include "sockets.h"
#include <parsi/parser.h>

char *IP_COORDINADOR, *IP_PLANIFICADOR, *ID;
int PUERTO_COORDINADOR, PUERTO_PLANIFICADOR;

int socketPlanificador, socketCoordinador;

char *programaAEjecutar;
bool terminar = false;

pthread_mutex_t binario_linea;

pthread_t hiloCoordinador, hiloPlanificador, hiloParser;

t_log* vg_logger;

/* Creo el logger de ESI*/

/*Creación de Logger*/
void crearLogger() {
	vg_logger = log_create("ESILog.log", "ESI", true, LOG_LEVEL_INFO);
}

void obtenerValoresArchivoConfiguracion() {
	t_config* arch = config_create(
			"/home/utnso/workspace/tp-2018-1c-Fail-system/ESI/esi.cfg");
	IP_COORDINADOR = string_duplicate(
			config_get_string_value(arch, "IP_COORDINADOR"));
	PUERTO_COORDINADOR = config_get_int_value(arch, "PUERTO_COORDINADOR");
	IP_PLANIFICADOR = string_duplicate(
			config_get_string_value(arch, "IP_PLANIFICADOR"));
	PUERTO_PLANIFICADOR = config_get_int_value(arch, "PUERTO_PLANIFICADOR");
	ID = string_duplicate(config_get_string_value(arch, "ID"));
	config_destroy(arch);
}

void imprimirArchivoConfiguracion() {
	printf("Configuración:\n"
			"IP_COORDINADOR=%s\n"
			"PUERTO_COORDINADOR=%d\n"
			"IP_PLANIFICADOR=%s\n"
			"PUERTO_PLANIFICADOR=%d\n"
			"ID=%s\n", IP_COORDINADOR, PUERTO_COORDINADOR, IP_PLANIFICADOR,
			PUERTO_PLANIFICADOR, ID);
	fflush(stdout);
}

void enviarHandshakeESI(int socketFD, char emisor[13]) {
	Paquete* paquete = malloc(TAMANIOHEADER + strlen(ID) + 1);
	Header header;
	header.tipoMensaje = ESHANDSHAKE;
	header.tamPayload = strlen(ID) + 1;
	paquete->Payload = malloc(strlen(ID) + 1);
	strcpy(paquete->Payload, ID);
	strcpy(header.emisor, emisor);
	paquete->header = header;
	EnviarPaquete(socketFD, paquete);
	free(paquete->Payload);
	free(paquete);
}

void escucharCoordinador(int socketFD, char emisor[13]) {
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteCliente(socketCoordinador, ESI, &paquete) > 0) {
		datos = paquete.Payload;
		switch (paquete.header.tipoMensaje) {
		//CASES
		}

		if (paquete.Payload != NULL) {
			free(paquete.Payload);
		}
	}
}

void escucharPlanificador(int socketFD, char emisor[13]) {
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteCliente(socketPlanificador, ESI, &paquete) > 0) {
		datos = paquete.Payload;
		switch (paquete.header.tipoMensaje) {
		case SIGUIENTELINEA: {
			pthread_mutex_unlock(&binario_linea);
		}
			break;

		case ABORTAR: {
			muerteEsi();
		}
			break;
		}

		if (paquete.Payload != NULL) {
			free(paquete.Payload);
		}
	}
}

void parsear() {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	void* datos;

	fp = fopen(programaAEjecutar, "r");
	if (fp == NULL) {
		perror("Error al abrir el archivo: ");
		muerteEsi();
	}

	while ((read = getline(&line, &len, fp)) != -1) {
		pthread_mutex_lock(&binario_linea);
		t_esi_operacion parsed = parse(line);
		if (parsed.valido) {
			switch (parsed.keyword) {
			case GET:
				datos = malloc(strlen(parsed.argumentos.GET.clave) + 1);
				strcpy(datos, parsed.argumentos.GET.clave);
				EnviarDatosTipo(socketCoordinador, ESI, datos,
						strlen(parsed.argumentos.GET.clave) + 1, GETCOORD);
				printf("GET\tclave: <%s>\n", parsed.argumentos.GET.clave);
//				log_info(vg_logger,
//						"ESI id: %s, se ejecutó operación GET, con clave: %s",
//						ID, parsed.argumentos.GET.clave);

				break;
			case SET:
				datos = malloc(
						strlen(parsed.argumentos.SET.clave)
								+ strlen(parsed.argumentos.SET.valor) + 2);
				strcpy(datos, parsed.argumentos.SET.clave);
				strcpy(datos + strlen(parsed.argumentos.SET.clave) + 1,
						parsed.argumentos.SET.valor);
				EnviarDatosTipo(socketCoordinador, ESI, datos,
						strlen(parsed.argumentos.SET.clave)
								+ strlen(parsed.argumentos.SET.valor) + 2,
						SETCOORD);
				printf("SET\tclave: <%s>\tvalor: <%s>\n",
						parsed.argumentos.SET.clave,
						parsed.argumentos.SET.valor);
//				log_info(vg_logger,
//						"ESI id: %s, se ejecutó operación SET, con clave: %s y valor: %s",
//						ID, parsed.argumentos.SET.clave,
//						parsed.argumentos.SET.valor);
				break;
			case STORE:
				datos = malloc(strlen(parsed.argumentos.STORE.clave) + 1);
				strcpy(datos, parsed.argumentos.STORE.clave);
				EnviarDatosTipo(socketCoordinador, ESI, datos,
						strlen(parsed.argumentos.STORE.clave) + 1, STORECOORD);
				printf("STORE\tclave: <%s>\n", parsed.argumentos.STORE.clave);
//				log_info(vg_logger,
//						"ESI id: %s, se ejecutó operación STORE, con clave: %s",
//						ID, parsed.argumentos.STORE.clave);
				break;
			default:
				fprintf(stderr, "No pude interpretar <%s>\n", line);
				muerteEsi();
			}

			destruir_operacion(parsed);
		} else {
			fprintf(stderr, "La linea <%s> no es valida\n", line);
			muerteEsi();
		}
	}

	fclose(fp);
	if (line)
		free(line);
	muerteEsi();

}

void muerteEsi() {
	close(socketCoordinador);

	EnviarDatosTipo(socketPlanificador, ESI, ID, strlen(ID) + 1, MUERTEESI);
	close(socketPlanificador);
	terminar = true;
}

int main(int argc, char* argv[]) {
	obtenerValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
	programaAEjecutar = argv[1];
	printf("El programa a ejecutar es %s\n", programaAEjecutar);
	fflush(stdout);
	pthread_mutex_init(&binario_linea, NULL);
	socketPlanificador = ConectarAServidorESI(PUERTO_PLANIFICADOR,
			IP_PLANIFICADOR, PLANIFICADOR, ESI, RecibirHandshake,
			enviarHandshakeESI);
	socketCoordinador = ConectarAServidorESI(PUERTO_COORDINADOR, IP_COORDINADOR,
	COORDINADOR, ESI, RecibirHandshake, enviarHandshakeESI);
	pthread_create(&hiloCoordinador, NULL, (void*) escucharCoordinador, NULL);
	pthread_create(&hiloPlanificador, NULL, (void*) escucharPlanificador, NULL);
	pthread_create(&hiloParser, NULL, (void*) parsear, NULL);
	while (!terminar){

	}
	pthread_join(hiloCoordinador, NULL);
	pthread_join(hiloPlanificador, NULL);
	pthread_join(hiloParser, NULL);
	return EXIT_SUCCESS;
}
