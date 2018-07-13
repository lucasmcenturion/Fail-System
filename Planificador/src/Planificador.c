#include "Consola.h"

char *IP, *ALGORITMO_PLANIFICACION, *IP_COORDINADOR;
int PUERTO, ESTIMACION_INICIAL, PUERTO_COORDINADOR, ALFA_ESTIMACION;
char **CLAVES_BLOQUEADAS;
int flag = 0;

pthread_mutex_t mutexOperaciones;
t_log* logger;

pthread_mutex_t ordenPlanificacionDetenida;
int socketCoordinador;
int tiempo = 0; //Controlar arribos

t_list* listaHilos;
bool end;
sem_t semaforoESI;
sem_t semPlanificacionDetenida;
/* Creo el logger de ESI*/

/*Creación de Logger*/
void crearLogger() {
	logger = log_create("PlanificadorLog.log", "PLANIFICADOR", true,
			LOG_LEVEL_INFO);
}

//COLAS
t_list *clavesBloqueadas;
t_list *LISTOS, *EJECUCION, *BLOQUEADOS, *TERMINADOS;

void inicializar() {
	clavesBloqueadas = list_create();
	LISTOS = list_create();
	EJECUCION = list_create();
	BLOQUEADOS = list_create();
	TERMINADOS = list_create();
	sem_init(&semaforoESI, 0, 0);
	sem_init(&semPlanificacionDetenida,1,0);
	pthread_mutex_init(&mutexOperaciones, NULL);
	pthread_mutex_init(&ordenPlanificacionDetenida,NULL);
}

void obtenerValoresArchivoConfiguracion() {
	t_config* arch = config_create("/home/utnso/workspace/tp-2018-1c-Fail-system/Planificador/planificador.cfg");
	IP = string_duplicate(config_get_string_value(arch, "IP"));
	PUERTO = config_get_int_value(arch, "PUERTO");
	ALGORITMO_PLANIFICACION = string_duplicate(config_get_string_value(arch, "ALGORITMO_PLANIFICACION"));
	ALFA_ESTIMACION = config_get_int_value(arch, "ALFA_ESTIMACION");
	ESTIMACION_INICIAL = config_get_int_value(arch, "ESTIMACION_INICIAL");
	IP_COORDINADOR = string_duplicate(config_get_string_value(arch, "IP_COORDINADOR"));
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
			"CLAVES_BLOQUEADAS=\n",
			IP, PUERTO, ALGORITMO_PLANIFICACION,
			ALFA_ESTIMACION, ESTIMACION_INICIAL,
			IP_COORDINADOR,PUERTO_COORDINADOR);

	string_iterate_lines(CLAVES_BLOQUEADAS, LAMBDA(void _(char* item1) {
		clavexEsi* cxe = malloc(sizeof(clavexEsi));
		cxe->idEsi = malloc(strlen("SYSTEM") + 1);
		cxe->clave = malloc(strlen(item1) + 1);
		cxe->valor = NULL;
		strcpy(cxe->idEsi, "SYSTEM");
		strcpy(cxe->clave, item1);
		list_add(clavesBloqueadas, cxe);
		printf("\t\t%s\n", item1);
	}));
	fflush(stdout);
}

void escuchaCoordinador() {
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteCliente(socketCoordinador, PLANIFICADOR, &paquete) > 0) {
		//sem_wait(&semaforoCoordinador);
		//sem_wait(&semaforoESI);
		datos = paquete.Payload;
		pthread_mutex_lock(&mutexOperaciones);
		switch (paquete.header.tipoMensaje) {
		case GETPLANI: {
			//Se fija si la clave que recibio está en la lista de claves bloqueadas
			if (list_any_satisfy(clavesBloqueadas, LAMBDA(bool _(clavexEsi* item1){return !strcmp(item1->clave, paquete.Payload);}))) {
				//Si está, bloquea al proceso ESI
				procesoEsi* esiABloquear = (procesoEsi*) list_remove_by_condition(EJECUCION, LAMBDA( bool _(procesoEsi* item1){ return !strcmp(item1->id, paquete.Payload + strlen(paquete.Payload)+1);}));
				esiBloqueado* esiBloqueado = malloc(sizeof(esiBloqueado));
				esiBloqueado->esi = esiABloquear;
				esiBloqueado->clave = malloc(strlen(paquete.Payload) + 1);
				strcpy(esiBloqueado->clave, paquete.Payload);
				list_add(BLOQUEADOS, esiBloqueado);
				planificar();
			} else {	//Sino, agrega la clave a claves bloqueadas
				clavexEsi* cxe = malloc(sizeof(clavexEsi));
				cxe->clave = malloc(strlen(paquete.Payload) + 1);
				cxe->valor = NULL;
				strcpy(cxe->clave, paquete.Payload);
				cxe->idEsi = malloc(strlen(paquete.Payload + strlen(cxe->clave)+1) + 1);
				strcpy(cxe->idEsi, paquete.Payload + strlen(cxe->clave) + 1);
				cxe->instancia = malloc(strlen(paquete.Payload + strlen(cxe->clave) + strlen(cxe->idEsi)+2) + 1);
				strcpy(cxe->instancia, paquete.Payload + strlen(cxe->clave) + strlen(cxe->idEsi)+2);
				list_add(clavesBloqueadas, cxe);
//				if(!strcmp(ALGORITMO_PLANIFICACION,"SJF-CD")){
//					procesoEsi* esiAEstarReady =(procesoEsi*) list_remove_by_condition(EJECUCION,LAMBDA(bool _(procesoEsi* item1){ return !strcmp(item1->id, paquete.Payload + strlen(paquete.Payload)+1);}));
//					list_add(LISTOS, esiAEstarReady);
//				}
				ChequearPlanificacionYSeguirEjecutando();
			}
			log_info(logger, "GET OK en Planificador");
		}
			break;

		case ABORTAR: { // VER SI CUANDO ABORTA LA OPERACION EL COORD. DEBERIAMOS HACER MUERTEESI PARA QUE LIBERE TODO OK
			procesoEsi* esiAAbortar = (procesoEsi*) list_find(EJECUCION, LAMBDA(bool _(procesoEsi* item1){ return !strcmp(item1->id, datos);}));
			EnviarDatosTipo(esiAAbortar->socket, PLANIFICADOR, NULL, 0, ABORTAR);
			log_info(logger, "Aborta ESI");
		}
			break;

		case CLAVESBLOQUEADAS: {
			datos = paquete.Payload;
			int tope = ((uint32_t*) datos)[0];
			datos += sizeof(uint32_t);
			for (int i = 0; i < tope; i++) {
				clavexEsi* cxe = list_get(clavesBloqueadas,i);
				cxe->instancia = malloc(strlen(datos)+1);
				strcpy(cxe->instancia, datos);
				datos += strlen(datos);
			}
		}
			break;

		case SETOKPLANI:{
			char* id, *value, *key, *instancia;
			id = paquete.Payload;
			value = paquete.Payload + strlen(id) + 1;
			key = paquete.Payload + strlen(id) + strlen(value) + 2;
			instancia = paquete.Payload + strlen(id) + strlen(value) + strlen(key) + 3;
			clavexEsi* cxe = list_find(clavesBloqueadas, LAMBDA(bool _(clavexEsi* item1) {return !strcmp(item1->clave,key);}));
			cxe->valor = malloc(strlen(value)+1);
			free(cxe->instancia);
			cxe->instancia = malloc(strlen(instancia)+1);
			strcpy(cxe->valor, value);
			strcpy(cxe->instancia, instancia);
//			if(!strcmp(ALGORITMO_PLANIFICACION,"SJF-CD")){
//				procesoEsi* esiAEstarReady =(procesoEsi*) list_remove_by_condition(EJECUCION,LAMBDA(bool _(procesoEsi* item1){ return !strcmp(item1->id, (char*)paquete.Payload);}));
//				list_add(LISTOS, esiAEstarReady);
//			}
			ChequearPlanificacionYSeguirEjecutando();
			log_info(logger, "SET OK en Planificador");
		}
			break;
		case STOREOKPLANI: {
			Desbloquear(datos, false);
			log_info(logger, " STORE OK al Planificador");
		}
			break;
		}
		pthread_mutex_unlock(&mutexOperaciones);

		if (paquete.Payload != NULL) {
			free(paquete.Payload);
		}
		sem_post(&semaforoESI);
	}
}

void CalcularEstimacion(procesoEsi* unEsi) {
	if(unEsi->rafagasRealesEjecutadas == 0){
		unEsi->rafagasEstimadas = ESTIMACION_INICIAL;
	}
	else{
		float a = ALFA_ESTIMACION/100.0;
		a = a * unEsi->rafagasRealesEjecutadas;
		float b = 1 - ALFA_ESTIMACION/100.0;
		b = b * unEsi->rafagasEstimadas;
		unEsi->rafagasEstimadas = a+b;
	}
//	printf("%s: ESTIMACION: %.4f\n", unEsi->id, unEsi->rafagasEstimadas);
//	fflush(stdout);
}

void EscucharESIyPlanificarlo(void* socket) {
	int socketFD = *(int*) socket;
	Paquete paquete;
	while (RecibirPaqueteServidorPlanificador(socketFD, PLANIFICADOR, &paquete) > 0) {
		if (!strcmp(paquete.header.emisor, ESI)) {
			switch (paquete.header.tipoMensaje) {
			case ESHANDSHAKE:
				printf("El proceso ESI es %s\n", (char*) paquete.Payload);
				log_info(logger, "El proceso ESI es %s\n", (char*) paquete.Payload);
				fflush(stdout);
				procesoEsi* nuevoEsi = malloc(sizeof(procesoEsi));
				nuevoEsi->id = malloc(strlen(paquete.Payload) + 1);
				nuevoEsi->socket = socketFD;
				nuevoEsi->rafagasRealesEjecutadas = 0;
				nuevoEsi->rafagasEstimadas = (float) ESTIMACION_INICIAL;
				strcpy(nuevoEsi->id, paquete.Payload);
				list_add(LISTOS, nuevoEsi);
				//ChequearPlanificacionYSeguirEjecutando();
				if(!planificacion_detenida){
					if (!strcmp(ALGORITMO_PLANIFICACION, "SJF-CD") || list_size(LISTOS) == 1){
						list_iterate(LISTOS, (void*) CalcularEstimacion);
						planificar();
					}
				}
				break;

			case MUERTEESI: {
				sem_wait(&semaforoESI);
				char* idEsiFinalizado = (char*) paquete.Payload;
				PasarESIMuertoAColaTerminados(idEsiFinalizado);
				ChequearPlanificacionYSeguirEjecutando();
				//libero claves bloqueadas por ese ESI
				while (list_any_satisfy(clavesBloqueadas, LAMBDA(bool _(clavexEsi* item1) {return !strcmp(item1->idEsi,idEsiFinalizado);}))) {
					clavexEsi* clavexEsiABorrar = list_remove_by_condition(clavesBloqueadas, LAMBDA(bool _(clavexEsi* item1) {return !strcmp(item1->idEsi,idEsiFinalizado);}));
					esiBloqueado* esiADesbloquear = list_remove_by_condition(BLOQUEADOS, LAMBDA(bool _(esiBloqueado* esiBloqueado ){ return !strcmp(esiBloqueado->clave, clavexEsiABorrar->clave);}));
					if (esiADesbloquear != NULL) {
						printf("Se desbloqueó el primer proceso ESI %s en la cola del recurso %s.\n",
								esiADesbloquear->esi->id,
								esiADesbloquear->clave);
						log_info(logger, "Se desbloqueó el primer proceso ESI %s en la cola del recurso %s.\n",
								esiADesbloquear->esi->id,
								esiADesbloquear->clave);
						procesoEsi* esiAPonerReady = malloc(sizeof(procesoEsi));
						esiAPonerReady->id = malloc(strlen(esiADesbloquear->esi->id) + 1);
						strcpy(esiAPonerReady->id, esiADesbloquear->esi->id);
						esiAPonerReady->rafagasEstimadas = esiADesbloquear->esi->rafagasEstimadas;
						esiAPonerReady->rafagasRealesEjecutadas = esiADesbloquear->esi->rafagasRealesEjecutadas;
						esiAPonerReady->socket = esiADesbloquear->esi->socket;
						clavexEsi* clavexEsiAAgregar = malloc(sizeof(clavexEsi));
						clavexEsiAAgregar->clave = malloc(strlen(clavexEsiABorrar->clave)+1);
						strcpy(clavexEsiAAgregar->clave, clavexEsiABorrar->clave);
						clavexEsiAAgregar->idEsi = malloc(strlen(esiADesbloquear->esi->id)+1);
						strcpy(clavexEsiAAgregar->idEsi, esiADesbloquear->esi->id);
						clavexEsiAAgregar->instancia = malloc(strlen(clavexEsiABorrar->instancia)+1);
						strcpy(clavexEsiAAgregar->instancia, clavexEsiABorrar->instancia);
						list_add(clavesBloqueadas, clavexEsiAAgregar);
						list_add(LISTOS, esiAPonerReady); //mandar func enviar a Listos
						if (!strcmp(ALGORITMO_PLANIFICACION, "SJF-CD") || list_size(LISTOS) == 1)
							list_iterate(LISTOS, (void*) CalcularEstimacion);
						free(esiADesbloquear->clave);
						free(esiADesbloquear->esi->id);
						free(esiADesbloquear->esi);
						free(esiADesbloquear);
						//if (!strcmp(ALGORITMO_PLANIFICACION, "SJF-CD") || list_size(LISTOS) == 1)
							//planificar();
					}
					free(clavexEsiABorrar->clave);
					free(clavexEsiABorrar->idEsi);
					free(clavexEsiABorrar->valor);
					free(clavexEsiABorrar->instancia);
					free(clavexEsiABorrar);
					ChequearPlanificacionYSeguirEjecutando();
				}
				/*PasarESIMuertoAColaTerminados(idEsiFinalizado); //Busco en que cola esta y paso el ESI a la cola de TERMINADOS
				if(list_size(BLOQUEADOS) > 0 )
					if(!strcmp(((esiBloqueado*)(list_get(BLOQUEADOS,0)))->esi->id, "Tiramisu"))
					{
						log_info(logger,"Ejecucion %s, %d, %d, %f", ((procesoEsi*) list_get(EJECUCION, 0))->id
								,((procesoEsi*) list_get(EJECUCION, 0))->socket,
								((procesoEsi*) list_get(EJECUCION, 0))->rafagasRealesEjecutadas,
								((procesoEsi*) list_get(EJECUCION, 0))->rafagasEstimadas);

					}
				ChequearPlanificacionYSeguirEjecutando();*/
				break;
			}
			}
		} else {
			perror("No es ningún proceso ESI.\n");
			if (paquete.Payload != NULL)
				free(paquete.Payload);
		}
	}
	close(socketFD);
}

void PasarESIMuertoAColaTerminados(char* idEsiFinalizado) {
	procesoEsi* esiTerminado;
	if (list_any_satisfy(BLOQUEADOS, LAMBDA(bool _(esiBloqueado* esiBloqueado ) {return !strcmp(esiBloqueado->esi->id,idEsiFinalizado);}))) {
		esiTerminado = list_remove_by_condition(BLOQUEADOS, LAMBDA( bool _(esiBloqueado* esiBloqueado ) {return !strcmp(esiBloqueado->esi->id,idEsiFinalizado);}));
		list_add(TERMINADOS, esiTerminado);
		log_info(logger, "El proceso ESI finalizado es %s\n", idEsiFinalizado);
	} else if (list_any_satisfy(LISTOS,	LAMBDA(	bool _(procesoEsi* esi ) {return !strcmp(esi->id,idEsiFinalizado);}))) {
		esiTerminado = list_remove_by_condition(LISTOS,	LAMBDA(bool _(procesoEsi* esi ) {return !strcmp(esi->id,idEsiFinalizado);}));
		list_add(TERMINADOS, esiTerminado);
		log_info(logger, "El proceso ESI finalizado es %s\n", idEsiFinalizado);
	} else if (list_any_satisfy(EJECUCION, LAMBDA( bool _(procesoEsi* esi ) {return !strcmp(esi->id,idEsiFinalizado);}))) {
		esiTerminado = list_remove_by_condition(EJECUCION, LAMBDA( bool _(procesoEsi* esi ) {return !strcmp(esi->id,idEsiFinalizado);}));
		list_add(TERMINADOS, esiTerminado);
		log_info(logger, "El proceso ESI finalizado es %s\n", idEsiFinalizado);
	}
	//log_info(logger, "El valor de la estimación del ESI %s es %.4f", esiTerminado->id, esiTerminado->rafagasEstimadas);
	//sem_post(&semaforoCoordinador);
}

bool ComparadorDeRafagas(procesoEsi* esi, procesoEsi* esiMenor) {
	return esi->rafagasEstimadas < esiMenor->rafagasEstimadas;
}

void ejecutarEsi() {
	//if(!planificacion_detenida){
		if (list_size(EJECUCION) != 0) {
			if(list_size(BLOQUEADOS) > 0 )
				if(!strcmp(((esiBloqueado*)(list_get(BLOQUEADOS,0)))->esi->id, "Tiramisu"))
				{
					log_info(logger,"Ejecucion %s, %d, %d, %f", ((procesoEsi*) list_get(EJECUCION, 0))->id
							,((procesoEsi*) list_get(EJECUCION, 0))->socket,
							((procesoEsi*) list_get(EJECUCION, 0))->rafagasRealesEjecutadas,
							((procesoEsi*) list_get(EJECUCION, 0))->rafagasEstimadas);

				}
			procesoEsi* esiAEjecutar = (procesoEsi*) list_get(EJECUCION, 0);
			++esiAEjecutar->rafagasRealesEjecutadas;
			EnviarDatosTipo(esiAEjecutar->socket, PLANIFICADOR, NULL, 0, SIGUIENTELINEA);
		}else {
			planificar();
		}
	/*}else{
		if(!strcmp(ALGORITMO_PLANIFICACION,"SJF-CD")){
			procesoEsi* esiAPonerReady = list_remove(EJECUCION,0);
			list_add(LISTOS,esiAPonerReady);
		}
	}*/
}

void ChequearPlanificacionYSeguirEjecutando() {
	if (flag==1){
		list_iterate(LISTOS, (void*) CalcularEstimacion);
		flag = 0;
	}
	if(!planificacion_detenida){
		if (!strcmp(ALGORITMO_PLANIFICACION, "SJF-CD")) {
			if(list_size(EJECUCION)>0){
				procesoEsi*aux=list_remove(EJECUCION,0);
				list_add(LISTOS,aux);
			}
			planificar();
		} else {
			ejecutarEsi();
		}
	}
}

void HacerSJF() {
//	list_iterate(LISTOS, (void*) CalcularEstimacion);
	t_list* listaAuxAOrdenar = list_duplicate(LISTOS);
	list_sort(listaAuxAOrdenar, (void*) ComparadorDeRafagas);
	procesoEsi* esiMenorEst = (procesoEsi*) list_get(listaAuxAOrdenar, 0);
	list_destroy(listaAuxAOrdenar);
	t_list* listaConLosMenoresOrdenadosPorOrdenDeLlegada = list_filter(LISTOS,LAMBDA(bool _(procesoEsi* item1) {return item1->rafagasEstimadas == esiMenorEst->rafagasEstimadas;}));
	procesoEsi* esiMenoryPrimeroEnLlegar = list_get(listaConLosMenoresOrdenadosPorOrdenDeLlegada,0);
	list_destroy(listaConLosMenoresOrdenadosPorOrdenDeLlegada);
	procesoEsi* esiAEjecutar = list_remove_by_condition(LISTOS,LAMBDA(bool _(procesoEsi* item1) {return !strcmp(item1->id,esiMenoryPrimeroEnLlegar->id);}));
    list_add(EJECUCION, esiAEjecutar);
    ejecutarEsi();
}

void planificar() {
	if(!planificacion_detenida){
		if (!list_is_empty(LISTOS)) {
			if (!strcmp(ALGORITMO_PLANIFICACION, "FIFO")) {
				procesoEsi* esiAEjecutar = (procesoEsi*) list_remove(LISTOS, 0);
				list_add(EJECUCION, esiAEjecutar);
				ejecutarEsi();
			} else if (!strcmp(ALGORITMO_PLANIFICACION, "SJF-SD")) {
				HacerSJF();
			} else if (!strcmp(ALGORITMO_PLANIFICACION, "SJF-CD")) {
				HacerSJF();
			} else if (!strcmp(ALGORITMO_PLANIFICACION, "HRRN")) {

			}
		}
	}

}

void EnviarHandshakePlani(int socketFD, char emisor[13]) {
	void *datos = malloc(0);
	int tamanio = 0, cant = 0;

	string_iterate_lines(CLAVES_BLOQUEADAS, LAMBDA(void _(char* item1) {
		datos = realloc(datos, tamanio + strlen(item1) + 1);
		strcpy(datos + tamanio, item1);
		tamanio += strlen(item1) + 1;
		cant++;
		free(item1);
	}));

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
	planificacion_detenida=false;
	crearLogger();
	socketCoordinador = ConectarAServidorPlanificador(PUERTO_COORDINADOR,
			IP_COORDINADOR, COORDINADOR,
			PLANIFICADOR, RecibirHandshake, EnviarHandshakePlani);
	pthread_t hiloCoordinador;
	pthread_create(&hiloCoordinador, NULL, (void*) escuchaCoordinador, NULL);
	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void*) consola, NULL);
	ServidorConcurrente(IP, PUERTO, PLANIFICADOR, &listaHilos, &end, EscucharESIyPlanificarlo);
	pthread_join(hiloConsola, NULL);
	pthread_join(hiloCoordinador, NULL);
	return EXIT_SUCCESS;
}
