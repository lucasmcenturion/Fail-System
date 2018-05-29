#include "Consola.h"

char *IP, *ALGORITMO_PLANIFICACION, *IP_COORDINADOR;
int PUERTO, ESTIMACION_INICIAL, PUERTO_COORDINADOR, ALFA_ESTIMACION;
char **CLAVES_BLOQUEADAS;

int socketCoordinador;
int tiempo = 0; //Controlar arribos

t_list* listaHilos;
bool end;

//COLAS

t_list *clavesBloqueadas;
t_list *LISTOS, *EJECUCION, *BLOQUEADOS, *TERMINADOS;

void inicializar() {
	clavesBloqueadas = list_create();
	LISTOS = list_create();
	EJECUCION = list_create();
	BLOQUEADOS = list_create();
	TERMINADOS = list_create();
}

void obtenerValoresArchivoConfiguracion() {
	t_config* arch =
			config_create(
					"/home/utnso/workspace/tp-2018-1c-Fail-system/Planificador/planificador.cfg");
	IP = string_duplicate(config_get_string_value(arch, "IP"));
	PUERTO = config_get_int_value(arch, "PUERTO");
	ALGORITMO_PLANIFICACION = string_duplicate(
			config_get_string_value(arch, "ALGORITMO_PLANIFICACION"));
	ALFA_ESTIMACION = config_get_int_value(arch, "ALFA_ESTIMACION");
	ESTIMACION_INICIAL = config_get_int_value(arch, "ESTIMACION_INICIAL");
	IP_COORDINADOR = string_duplicate(
			config_get_string_value(arch, "IP_COORDINADOR"));
	PUERTO_COORDINADOR = config_get_int_value(arch, "PUERTO_COORDINADOR");
	CLAVES_BLOQUEADAS = config_get_array_value(arch, "CLAVES_BLOQUEADAS");
	config_destroy(arch);
}

void imprimirArchivoConfiguracion() {
	printf("Configuración:\n"
			"IP=%s\n"
			"PUERTO=%d\n"
			"ALGORITMO_PLANIFICACION=%s\n"
			"ALFA_ESTIMACION=%d\n"
			"ESTIMACION_INICIAL=%d\n"
			"IP_COORDINADOR=%s\n"
			"PUERTO_COORDINADOR=%d\n"
			"CLAVES_BLOQUEADAS=\n", IP, PUERTO, ALGORITMO_PLANIFICACION,
			ALFA_ESTIMACION, ESTIMACION_INICIAL, IP_COORDINADOR,
			PUERTO_COORDINADOR);

	string_iterate_lines(CLAVES_BLOQUEADAS,
			LAMBDA(
					void _(char* item1) { clavexEsi* cxe = malloc(sizeof(clavexEsi)); cxe->idEsi = malloc(strlen("SYSTEM")+1); cxe->clave = malloc(strlen(item1)+1); cxe->borrar = false; strcpy(cxe->idEsi, "SYSTEM"); strcpy(cxe->clave, item1); list_add(clavesBloqueadas, cxe); printf("\t\t%s\n",item1); }));
	fflush(stdout);
}

void escuchaCoordinador() {
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteCliente(socketCoordinador, PLANIFICADOR, &paquete) > 0) {
		datos = paquete.Payload;
		switch (paquete.header.tipoMensaje) {
		case GETPLANI: {
			//Se fija si la clave que recibio está en la lista de claves bloqueadas
			if (list_any_satisfy(clavesBloqueadas,
					LAMBDA(
							bool _(clavexEsi* item1){ return !strcmp(item1->clave, paquete.Payload);}))) {
				//Si está, bloquea al proceso ESI
				procesoEsi* esiABloquear =
						(procesoEsi*) list_remove_by_condition(EJECUCION,
								LAMBDA(
										bool _(procesoEsi* item1){ return !strcmp(item1->id, paquete.Payload + strlen(paquete.Payload)+1);}));
				esiBloqueado* esiBloqueado = malloc(sizeof(esiBloqueado));
				esiBloqueado->esi = esiABloquear;
				esiBloqueado->clave = malloc(strlen(paquete.Payload) + 1);
				strcpy(esiBloqueado->clave, paquete.Payload);
				list_add(BLOQUEADOS, esiBloqueado);
				planificar();
			} else {	//Sino, agrega la clave a claves bloqueadas
				clavexEsi* cxe = malloc(sizeof(clavexEsi));
				cxe->idEsi = malloc(
						strlen(paquete.Payload + strlen(paquete.Payload) + 1)
								+ 1);
				cxe->clave = malloc(strlen(paquete.Payload) + 1);
				cxe->borrar = false;
				strcpy(cxe->idEsi,
						paquete.Payload + strlen(paquete.Payload) + 1);
				strcpy(cxe->clave, paquete.Payload);
				list_add(clavesBloqueadas, cxe);
//				procesoEsi* esiAEstarReady =
//						(procesoEsi*) list_remove_by_condition(EJECUCION,
//								LAMBDA(
//										bool _(procesoEsi* item1){ return !strcmp(item1->id, paquete.Payload + strlen(paquete.Payload)+1);}));
//				list_add(LISTOS, esiAEstarReady);
				ChequearPlanificacionYSeguirEjecutando();
			}

		}
			break;
			/* PARA EL CASO DEL STORE
			 case MENSAJEQUEMEMANDEJULIPARASTORE:
			 {
			 //Se fija si la clave que recibio está en la lista de claves bloqueadas
			 if(list_any_satisfy(clavesBloqueadas, LAMBDA(bool _(clavexEsi* item1){ return !strcmp(item1->clave, paquete.Payload);})))
			 {
			 //Si está, bloquea al proceso ESI
			 procesoEsi* esiABloquear = (procesoEsi*) list_remove_by_condition(EJECUCION, LAMBDA(bool _(procesoEsi* item1){ return !strcmp(item1->id, paquete.Payload + strlen(paquete.Payload)+1);}));
			 esiBloqueado* esiBloqueado = malloc(sizeof(esiBloqueado));
			 esiBloqueado->esi = esiABloquear;
			 esiBloqueado->clave = malloc(strlen(paquete.Payload)+1);
			 strcpy(esiBloqueado->clave, paquete.Payload);
			 list_add(BLOQUEADOS, esiBloqueado);
			 }
			 else
			 {
			 //Sino, agrega la clave a claves bloqueadas
			 clavexEsi* cxe = malloc(sizeof(clavexEsi));
			 cxe->idEsi = malloc(strlen(paquete.Payload + strlen(paquete.Payload) + 1) + 1);
			 cxe->clave = malloc(strlen(paquete.Payload) + 1);
			 cxe->borrar = false;
			 strcpy(cxe->idEsi, paquete.Payload + strlen(paquete.Payload) + 1);
			 strcpy(cxe->clave, paquete.Payload);
			 list_add(clavesBloqueadas, cxe);
			 procesoEsi* esiAEstarReady = (procesoEsi*) list_remove_by_condition(EJECUCION, LAMBDA(bool _(procesoEsi* item1){ return !strcmp(item1->id, paquete.Payload + strlen(paquete.Payload)+1);}));
			 list_add(LISTOS, esiAEstarReady);
			 //if (!strcmp(ALGORITMO_PLANIFICACION,"SJF/CD"))
			 //	planificar();
			 }

			 }
			 break;
			 */

		case ABORTAR: { // VER SI CUANDO ABORTA LA OPERACION EL COORD. DEBERIAMOS HACER MUERTEESI PARA QUE LIBERE TODO OK
			procesoEsi* esiAAbortar =
					(procesoEsi*) list_remove_by_condition(EJECUCION,
							LAMBDA(
									bool _(procesoEsi* item1){ return !strcmp(item1->id, datos);}));
			EnviarDatosTipo(esiAAbortar->socket, PLANIFICADOR, NULL, 0,
					ABORTAR);
			list_add(TERMINADOS, esiAAbortar);
		}
			break;
		case SETOKPLANI:
			ChequearPlanificacionYSeguirEjecutando();

			break;
		case STOREOKPLANI:
			ChequearPlanificacionYSeguirEjecutando();

			break;
		}

		if (paquete.Payload != NULL) {
			free(paquete.Payload);
		}
	}
}

void accion(void* socket) {
	int socketFD = *(int*) socket;
	Paquete paquete;
	while (RecibirPaqueteServidorPlanificador(socketFD, PLANIFICADOR, &paquete)
			> 0) {
		if (!strcmp(paquete.header.emisor, ESI)) {
			switch (paquete.header.tipoMensaje) {
			case ESHANDSHAKE:
				printf("El proceso ESI es %s\n", (char*) paquete.Payload);
				fflush(stdout);
				procesoEsi* nuevoEsi = malloc(sizeof(procesoEsi));
				nuevoEsi->id = malloc(strlen(paquete.Payload) + 1);
				nuevoEsi->socket = socketFD;
				strcpy(nuevoEsi->id, paquete.Payload);
				list_add(LISTOS, nuevoEsi);
				if (!strcmp(ALGORITMO_PLANIFICACION, "SJF/CD")
						|| list_size(LISTOS) == 1)
					planificar();
				break;

			case MUERTEESI: {
				char* idEsiFinalizado = (char*) paquete.Payload;
				printf("El proceso ESI finalizado es %s\n", idEsiFinalizado);
				fflush(stdout);
				//libero claves bloqueadas por ese ESI
				while (list_any_satisfy(clavesBloqueadas,
						LAMBDA(
								bool _(clavexEsi* item1) {return !strcmp(item1->idEsi,idEsiFinalizado);}))) {
					clavexEsi* clavexEsiABorrar =
							list_remove_by_condition(clavesBloqueadas,
									LAMBDA(
											bool _(clavexEsi* item1) {return !strcmp(item1->idEsi,idEsiFinalizado);}));
					esiBloqueado* esiADesbloquear =
							list_remove_by_condition(BLOQUEADOS,
									LAMBDA(
											bool _(esiBloqueado* esiBloqueado ){ return !strcmp(esiBloqueado->clave, clavexEsiABorrar->clave);}));
					printf(
							"Se desbloqueó el primer proceso ESI %s en la cola del recurso %s.\n",
							esiADesbloquear->esi->id, esiADesbloquear->clave);
					list_add(LISTOS, esiADesbloquear->esi);
					free(esiADesbloquear->clave);
					free(clavexEsiABorrar->clave);
					free(clavexEsiABorrar->idEsi);
					free(clavexEsiABorrar);
				}
				PasarESIMuertoAColaTerminados(idEsiFinalizado); //Busco en que cola esta y paso el ESI a la cola de TERMINADOS
				break;

			}

			}
		} else {
			perror("No es ningún proceso ESI.\n");
			if (paquete.Payload != NULL)
				free(paquete.Payload);
		}
		close(socketFD);
	}
}

void PasarESIMuertoAColaTerminados(char* idEsiFinalizado) {
	if (list_any_satisfy(BLOQUEADOS,
			LAMBDA(
					bool _(esiBloqueado* esiBloqueado ) {return !strcmp(esiBloqueado->esi->id,idEsiFinalizado);}))) {

		procesoEsi* esiTerminado =
				list_remove_by_condition(BLOQUEADOS,
						LAMBDA(
								bool _(esiBloqueado* esiBloqueado ) {return !strcmp(esiBloqueado->esi->id,idEsiFinalizado);}));
		list_add(TERMINADOS, esiTerminado);
	} else if (list_any_satisfy(LISTOS,
			LAMBDA(
					bool _(procesoEsi* esi ) {return !strcmp(esi->id,idEsiFinalizado);}))) {

		procesoEsi* esiTerminado =
				list_remove_by_condition(LISTOS,
						LAMBDA(
								bool _(procesoEsi* esi ) {return !strcmp(esi->id,idEsiFinalizado);}));
		list_add(TERMINADOS, esiTerminado);
	} else if (list_any_satisfy(EJECUCION,
			LAMBDA(
					bool _(procesoEsi* esi ) {return !strcmp(esi->id,idEsiFinalizado);}))) {

		procesoEsi* esiTerminado =
				list_remove_by_condition(EJECUCION,
						LAMBDA(
								bool _(procesoEsi* esi ) {return !strcmp(esi->id,idEsiFinalizado);}));
		list_add(TERMINADOS, esiTerminado);

	}
}

procesoEsi* CalcularEstimacion(procesoEsi* unEsi) {
	unEsi->rafagasEstimadas = (ALFA_ESTIMACION * ESTIMACION_INICIAL)
			+ ((1 - ALFA_ESTIMACION) * (unEsi->rafagasRealesEjecutadas));
	return unEsi;
}

bool ComparadorDeRafagas(procesoEsi* esi, procesoEsi* esiMenor) {
	return esi->rafagasEstimadas < esiMenor->rafagasEstimadas;
}

void ejecutarEsi() {
	procesoEsi* esiAEjecutar = (procesoEsi*) list_get(EJECUCION, 0);
	EnviarDatosTipo(esiAEjecutar->socket, PLANIFICADOR, NULL, 0,
			SIGUIENTELINEA);

}
void ChequearPlanificacionYSeguirEjecutando() {
	if (!strcmp(ALGORITMO_PLANIFICACION, "SJF/CD")) {
		planificar();
	} else {
		ejecutarEsi();
	}
}

void planificar() {
	if (!planificacion_detenida) {
		if (!list_is_empty(LISTOS)) {
			if (!strcmp(ALGORITMO_PLANIFICACION, "FIFO")) {
				procesoEsi* esiAEjecutar = (procesoEsi*) list_remove(LISTOS, 0);
				list_add(EJECUCION, esiAEjecutar);
				ejecutarEsi();

			} else if (!strcmp(ALGORITMO_PLANIFICACION, "SJF/SD")) {
				//AUX: lista ordenada de esis por estimacion de rafaga
				t_list* AUX = list_map(LISTOS, (void*) CalcularEstimacion);
				list_sort(AUX, (void*) ComparadorDeRafagas);

				procesoEsi* esiMenorEst = (procesoEsi*) list_get(AUX, 0);
				//si hay ESIs con estimaciones repetidas
				//AUX2: lista ordenada por orden de llegada de esis con igual estimacion
				t_list* AUX2 = list_create();
				for (int x = 0; x < list_size(AUX); x++) {
					procesoEsi* unEsi = (procesoEsi*) list_get(AUX, x);
					if (unEsi->rafagasEstimadas
							== esiMenorEst->rafagasEstimadas) {
						list_add(AUX2, unEsi);
					}
				}
				procesoEsi* esiAEjecutar = (procesoEsi*) list_remove(AUX2, 0);
				list_add(EJECUCION, esiAEjecutar);
				ejecutarEsi();
//			} else if (!strcmp(ALGORITMO_PLANIFICACION, "SJF/CD")) {

			} else if (!strcmp(ALGORITMO_PLANIFICACION, "HRRN")) {

			}
		}
	}
}

void EnviarHandshakePlani(int socketFD, char emisor[13]) {
	void *datos = malloc(0);
	int tamanio = 0, cant = 0;

	string_iterate_lines(CLAVES_BLOQUEADAS, LAMBDA(void _(char* item1) {
		datos = realloc(datos, tamanio + strlen(item1) + 1)
		;
		strcpy(datos + tamanio, item1)
		;
		tamanio += strlen(item1) + 1
		;
		cant++
		;
		free(item1)
		;
	}
	));

	Paquete* paquete = malloc(
	TAMANIOHEADER + sizeof(int) + tamanio);
	Header header;
	header.tipoMensaje = ESHANDSHAKE;

	header.tamPayload = tamanio + sizeof(int);
	paquete->Payload = malloc(sizeof(int) + tamanio);

	((int*) paquete->Payload)[0] = cant;
	memcpy(paquete->Payload + sizeof(int), datos, tamanio);
	free(datos);

	strcpy(header.emisor, emisor);
	paquete->header = header;

	EnviarPaquete(socketFD, paquete);

	free(paquete->Payload);
	free(paquete);
	free(CLAVES_BLOQUEADAS);
}

int main(void) {
	inicializar();
	obtenerValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
	planificacion_detenida = false;
	socketCoordinador = ConectarAServidorPlanificador(PUERTO_COORDINADOR,
			IP_COORDINADOR, COORDINADOR,
			PLANIFICADOR, RecibirHandshake, EnviarHandshakePlani);
	pthread_t hiloCoordinador;
	pthread_create(&hiloCoordinador, NULL, (void*) escuchaCoordinador,
	NULL);
	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void*) consola, NULL);
	pthread_t hiloPlanificador;
	ServidorConcurrente(IP, PUERTO, PLANIFICADOR, &listaHilos, &end, accion);
	pthread_join(hiloConsola, NULL);
	pthread_join(hiloPlanificador, NULL);
	pthread_join(hiloCoordinador, NULL);
	return EXIT_SUCCESS;
}
