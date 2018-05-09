#include "Consola.h"

char *IP, *ALGORITMO_PLANIFICACION, *IP_COORDINADOR;
int PUERTO, ESTIMACION_INICIAL, PUERTO_COORDINADOR;
char **CLAVES_BLOQUEADAS;


int socketCoordinador;

t_list* listaHilos;
bool end;

//COLAS

t_list *clavesBloqueadas;
t_list *LISTOS, *EJECUCION, *BLOQUEADOS, *TERMINADOS;


void inicializar(){
	clavesBloqueadas = list_create();
	LISTOS = list_create();
	EJECUCION = list_create();
	BLOQUEADOS = list_create();
	TERMINADOS = list_create();
}

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

	string_iterate_lines(CLAVES_BLOQUEADAS, LAMBDA(void _(char* item1)
		{
		    list_add(clavesBloqueadas, item1);
			printf("\t\t   %s\n", item1);
		}));
	free(CLAVES_BLOQUEADAS);
	fflush(stdout);
}

void escuchaCoordinador(){
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteCliente(socketCoordinador, PLANIFICADOR, &paquete)>0){
		datos=paquete.Payload;
		switch(paquete.header.tipoMensaje)
		{
			case BLOQUEODECLAVE:
			{
				/*LO QUE SEA QUE ME MANDE EL COORDINADOR PARA SABER QUE SE BLOQUEO UNA CLAVE (GET)*/
				list_add(clavesBloqueadas, datos);

			}
			break;

			case DESBLOQUEODECLAVE:
			{
				/*LO QUE SEA QUE ME MANDE EL COORDINADOR PARA SABER QUE SE DESBLOQUEO UNA CLAVE (STORE)*/
				list_remove_and_destroy_by_condition(clavesBloqueadas, LAMBDA(bool _(char* item)
				{
					return !strcmp(item, datos);
				}),
				free);

			}
			break;
		}

		if (paquete.Payload != NULL){
			free(paquete.Payload);
		}
	}
}

void accion(void* socket) {
	int socketFD = *(int*) socket;
	Paquete paquete;
	while (RecibirPaqueteServidorPlanificador(socketFD, PLANIFICADOR, &paquete) > 0) {
		if (!strcmp(paquete.header.emisor, ESI)) {
			switch(paquete.header.tipoMensaje)
			{
				case ESHANDSHAKE:
					printf("El proceso ESI es %s\n", (char*)paquete.Payload);
					fflush(stdout);
					procesoEsi* nuevoEsi =  malloc(sizeof(procesoEsi));
					nuevoEsi->id = malloc(strlen(paquete.Payload) + 1);
					strcpy(nuevoEsi->id, paquete.Payload);
					list_add(LISTOS, nuevoEsi);
					break;
			}
		}
		else
			perror("No es ningún proceso ESI.\n");
		if (paquete.Payload != NULL)
			free(paquete.Payload);
	}
	close(socketFD);
}


void planificar()
{
	while (!planificacion_detenida)
	{
			if (!strcmp(ALGORITMO_PLANIFICACION, "FIFO"))
			{
				procesoEsi* esiAEjecutar = (procesoEsi*) list_remove(LISTOS, 0);
				list_add(EJECUCION, esiAEjecutar);
			}
			else if (!strcmp(ALGORITMO_PLANIFICACION, "SJF"))
			{

			}
			else if (!strcmp(ALGORITMO_PLANIFICACION, "HRRN"))
			{

			}
	}
}

int main(void) {
	inicializar();
	obtenerValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
	planificacion_detenida = false;
	socketCoordinador = ConectarAServidor(PUERTO_COORDINADOR, IP_COORDINADOR, COORDINADOR, PLANIFICADOR, RecibirHandshake);
	pthread_t hiloCoordinador;
	pthread_create(&hiloCoordinador, NULL, (void*) escuchaCoordinador, NULL);
	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void*) consola, NULL);
	pthread_t hiloPlanificador;
	pthread_create(&hiloPlanificador, NULL, (void*)planificar, NULL);
	ServidorConcurrente(IP,PUERTO, PLANIFICADOR, &listaHilos, &end, accion);
	pthread_join(hiloConsola, NULL);
	return EXIT_SUCCESS;
}
