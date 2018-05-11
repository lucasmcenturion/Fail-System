#include "sockets.h"
#include <parsi/parser.h>

char *IP_COORDINADOR, *IP_PLANIFICADOR, *ID;
int PUERTO_COORDINADOR, PUERTO_PLANIFICADOR;

int socketPlanificador, socketCoordinador;

char *programaAEjecutar;

pthread_mutex_t binario_linea;


void obtenerValoresArchivoConfiguracion() {
	t_config* arch = config_create("/home/utnso/workspace/tp-2018-1c-Fail-system/ESI/esi.cfg");
	IP_COORDINADOR = string_duplicate(config_get_string_value(arch, "IP_COORDINADOR"));
	PUERTO_COORDINADOR = config_get_int_value(arch, "PUERTO_COORDINADOR");
	IP_PLANIFICADOR = string_duplicate(config_get_string_value(arch, "IP_PLANIFICADOR"));
	PUERTO_PLANIFICADOR = config_get_int_value(arch, "PUERTO_PLANIFICADOR");
	ID = string_duplicate(config_get_string_value(arch, "ID"));
	config_destroy(arch);
}

void imprimirArchivoConfiguracion(){
	printf(
				"Configuración:\n"
				"IP_COORDINADOR=%s\n"
				"PUERTO_COORDINADOR=%d\n"
				"IP_PLANIFICADOR=%s\n"
				"PUERTO_PLANIFICADOR=%d\n"
				"ID=%s\n",
				IP_COORDINADOR,
				PUERTO_COORDINADOR,
				IP_PLANIFICADOR,
				PUERTO_PLANIFICADOR,
				ID
				);
	fflush(stdout);
}

void enviarHandshakeESI(int socketFD,char emisor[13]){
	Paquete* paquete = malloc(TAMANIOHEADER+strlen(ID)+1);
	Header header;
	header.tipoMensaje = ESHANDSHAKE;
	header.tamPayload = strlen(ID)+1;
	paquete->Payload=malloc(strlen(ID)+1);
	strcpy(paquete->Payload,ID);
	strcpy(header.emisor, emisor);
	paquete->header = header;
	EnviarPaquete(socketFD, paquete);
	free(paquete->Payload);
	free(paquete);
}

void escucharCoordinador(int socketFD,char emisor[13]){
	Paquete paquete;
		void* datos;
		while (RecibirPaqueteCliente(socketCoordinador, ESI, &paquete)>0){
			datos=paquete.Payload;
			switch(paquete.header.tipoMensaje)
			{
				//CASES
			}

			if (paquete.Payload != NULL){
				free(paquete.Payload);
			}
		}
}

void escucharPlanificador(int socketFD,char emisor[13]){
	Paquete paquete;
		void* datos;
		while (RecibirPaqueteCliente(socketPlanificador, ESI, &paquete)>0){
			datos=paquete.Payload;
			switch(paquete.header.tipoMensaje)
			{
				case SIGUIENTELINEA:
				{
					pthread_mutex_unlock(&binario_linea);
				}
				break;

				case REPETIRLINEA:
				{
					//ALGO
				}
				break;
			}

			if (paquete.Payload != NULL){
				free(paquete.Payload);
			}
		}
}

int main(int argc, char* argv[]) {
	obtenerValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
	programaAEjecutar = argv[1];
	printf("El programa a ejecutar es %s\n", programaAEjecutar);
	fflush(stdout);
	pthread_mutex_init(&binario_linea, NULL);
	socketPlanificador = ConectarAServidorESI(PUERTO_PLANIFICADOR, IP_PLANIFICADOR, PLANIFICADOR, ESI, RecibirHandshake, enviarHandshakeESI);
	socketCoordinador = ConectarAServidorESI(PUERTO_COORDINADOR, IP_COORDINADOR, COORDINADOR, ESI, RecibirHandshake, enviarHandshakeESI);
	pthread_t hiloCoordinador;
	pthread_create(&hiloCoordinador, NULL, (void*) escucharCoordinador, NULL);
	pthread_t hiloPlanificador;
	pthread_create(&hiloPlanificador, NULL, (void*) escucharPlanificador, NULL);
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen(programaAEjecutar, "r");
	if (fp == NULL){
		perror("Error al abrir el archivo: ");
		exit(EXIT_FAILURE);
	}

	while ((read = getline(&line, &len, fp)) != -1) {
		pthread_mutex_lock(&binario_linea);
		t_esi_operacion parsed = parse(line);
		void* datos;
		if(parsed.valido){
			switch(parsed.keyword){
				case GET:

					printf("GET\tclave: <%s>\n", parsed.argumentos.GET.clave);
					EnviarDatosTipo(socketCoordinador, ESI, datos, tamanio, GETCOORD);
					break;
				case SET:
					printf("SET\tclave: <%s>\tvalor: <%s>\n", parsed.argumentos.SET.clave, parsed.argumentos.SET.valor);
					EnviarDatosTipo(socketCoordinador, ESI, datos, tamanio, SETCOORD);

					break;
				case STORE:
					printf("STORE\tclave: <%s>\n", parsed.argumentos.STORE.clave);
					EnviarDatosTipo(socketCoordinador, ESI, datos, tamanio, STORECOORD);
					break;
				default:
					fprintf(stderr, "No pude interpretar <%s>\n", line);
					exit(EXIT_FAILURE);
			}

			destruir_operacion(parsed);
		} else {
			fprintf(stderr, "La linea <%s> no es valida\n", line);
			exit(EXIT_FAILURE);
		}
	}

	fclose(fp);
	if (line)
		free(line);
	return EXIT_SUCCESS;
}
