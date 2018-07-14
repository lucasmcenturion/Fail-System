#include "Consola.h"

bool planificacion_detenida;
t_list *LISTOS, *EJECUCION, *BLOQUEADOS, *clavesBloqueadas;
t_log* logger;
sem_t semPlanificacionDetenida;
int flag;
int tiempoActual;

void PausarContinuar() {
	planificacion_detenida = !planificacion_detenida;
	if (planificacion_detenida) {
		log_info(logger, "La planificación se detuvo.\n");
	} else {
		log_info(logger, "La planificacion se renaudó.\n");
		planificar();
	}
}

void Bloquear(char* clave, char* id) {
	procesoEsi* esiABloquear = list_remove_by_condition(LISTOS,
			LAMBDA(bool _(procesoEsi* item1){ return !strcmp(item1->id, id);}));
	if (esiABloquear == NULL) {
		//si no lo encuentra en listos, buscarlo en ejecucion
		esiABloquear =
				list_remove_by_condition(EJECUCION,
						LAMBDA(
								bool _(procesoEsi* item1){ return !strcmp(item1->id, id);}));
		if (esiABloquear == NULL) {
			//si no lo encuentra acá, no existe el esi que se pretende bloquear.
			log_info(logger,
					"El ESI de id %s no se puede bloquear ya que no existe ningún ESI con ese id.",
					id);
		} else {
			//lo encontró en ejecucion, lo agrega a bloqueados.
			esiBloqueado* esiBloqueado = malloc(sizeof(esiBloqueado));
			esiBloqueado->esi = esiABloquear;
			esiBloqueado->clave = malloc(strlen(clave) + 1);
			strcpy(esiBloqueado->clave, clave);
			list_add(BLOQUEADOS, esiBloqueado);
			clavexEsi* claveXEsiABloquear = malloc(sizeof(clavexEsi));
			claveXEsiABloquear->clave = malloc(strlen(clave) + 1);
			strcpy(claveXEsiABloquear->clave, clave);
			claveXEsiABloquear->idEsi = malloc(strlen(id) + 1);
			strcpy(claveXEsiABloquear->idEsi, id);
			list_add(clavesBloqueadas, claveXEsiABloquear);
			log_info(logger,
					"Se bloqueó el proceso ESI de id %s, en la cola del recurso %s.\n",
					clave, id);
		}
	} else {
		//lo encontró en listos, lo agrega a bloqueados.
		esiBloqueado* esiBloqueado = malloc(sizeof(esiBloqueado));
		esiBloqueado->esi = esiABloquear;
		esiBloqueado->clave = malloc(strlen(clave) + 1);
		strcpy(esiBloqueado->clave, clave);
		list_add(BLOQUEADOS, esiBloqueado);
		clavexEsi* claveXEsiABloquear = malloc(sizeof(clavexEsi));
		claveXEsiABloquear->clave = malloc(strlen(clave) + 1);
		strcpy(claveXEsiABloquear->clave, clave);
		claveXEsiABloquear->idEsi = malloc(strlen(id) + 1);
		strcpy(claveXEsiABloquear->idEsi, id);
		list_add(clavesBloqueadas, claveXEsiABloquear);
		log_info(logger,
				"Se bloqueó el proceso ESI de id %s, en la cola del recurso %s.\n",
				clave, id);
	}
	fflush(stdout);
}

void Desbloquear(char* clave, bool flagPrint) {
	clavexEsi* clavexEsiABorrar =
			list_remove_by_condition(clavesBloqueadas,
					LAMBDA(
							bool _(clavexEsi* item1){ return !strcmp(item1->clave, clave);}));
	if (clavexEsiABorrar != NULL) {
		esiBloqueado* esiDesbloqueado =
				list_remove_by_condition(BLOQUEADOS,
						LAMBDA(
								bool _(esiBloqueado* item1){ return !strcmp(item1->clave, clavexEsiABorrar->clave);}));
		if (esiDesbloqueado != NULL) {
			if (flagPrint)
				log_info(logger,
						"Se desbloqueó el primer proceso ESI %s en la cola del recurso %s.\n",
						esiDesbloqueado->esi->id, esiDesbloqueado->clave);
			tiempoActual--;
			procesoEsi* esiAPonerReady = malloc(sizeof(procesoEsi));
			esiAPonerReady->id = malloc(strlen(esiDesbloqueado->esi->id) + 1);
			strcpy(esiAPonerReady->id, esiDesbloqueado->esi->id);
			esiAPonerReady->rafagasEstimadas =
					esiDesbloqueado->esi->rafagasEstimadas;
			esiAPonerReady->rafagasRealesEjecutadas =
					esiDesbloqueado->esi->rafagasRealesEjecutadas;
			esiAPonerReady->socket = esiDesbloqueado->esi->socket;
			esiAPonerReady->tiempoDeLlegada = tiempoActual;
			clavexEsi* clavexEsiAAgregar = malloc(sizeof(clavexEsi));
			clavexEsiAAgregar->clave = malloc(strlen(clave) + 1);
			strcpy(clavexEsiAAgregar->clave, clave);
			clavexEsiAAgregar->idEsi = malloc(
					strlen(esiDesbloqueado->esi->id) + 1);
			strcpy(clavexEsiAAgregar->idEsi, esiDesbloqueado->esi->id);
			clavexEsiAAgregar->instancia = malloc(
					strlen(clavexEsiABorrar->instancia) + 1);
			strcpy(clavexEsiAAgregar->instancia, clavexEsiABorrar->instancia);
			list_add(clavesBloqueadas, clavexEsiAAgregar);
			list_add(LISTOS, esiAPonerReady);
			flag = 1;
			//		if (!strcmp(ALGORITMO_PLANIFICACION, "SJF-CD") || list_size(LISTOS) == 1){
			//			list_iterate(LISTOS, (void*) CalcularEstimacion);
			//		}
			free(esiDesbloqueado->clave);
			free(esiDesbloqueado->esi->id);
			free(esiDesbloqueado->esi);
			free(esiDesbloqueado);
		}
		if (clavexEsiABorrar) {
			free(clavexEsiABorrar->idEsi);
			free(clavexEsiABorrar->valor);
			free(clavexEsiABorrar->instancia);
			free(clavexEsiABorrar);
		}
		ChequearPlanificacionYSeguirEjecutando();
	} else {
		log_info(logger, "La clave %s no se encontraba bloqueada.", clave);
	}

}

void Listar(char* recurso) {
	if (list_any_satisfy(BLOQUEADOS,
			LAMBDA(
					bool _(esiBloqueado* item1) { return !strcmp(item1->clave, recurso);

					}))) {
		log_info(logger,
				"A continuación, se listan los procesos bloqueados esperando el recurso %s:\n",
				recurso);
		list_iterate(BLOQUEADOS,
				LAMBDA(
						void _(esiBloqueado* item1) { if (!strcmp(item1->clave, recurso)) { printf("\t\t- %s\n", item1->esi->id); fflush(stdout); } }));
	} else
		log_info(logger, "No hay procesos bloqueados esperando el recurso %s\n",
				recurso);
}

void Kill(char* id) {
	procesoEsi* esiAMatar = (procesoEsi*) list_find(EJECUCION,
			LAMBDA(bool _(procesoEsi* item1){ return !strcmp(item1->id, id);}));
	if (esiAMatar == NULL) {
		esiAMatar =
				(procesoEsi*) list_find(LISTOS,
						LAMBDA(
								bool _(procesoEsi* item1){ return !strcmp(item1->id, id);}));
		if (esiAMatar == NULL) {
			esiAMatar =
					((esiBloqueado*) list_find(BLOQUEADOS,
							LAMBDA(
									bool _(esiBloqueado* item1){ return !strcmp(item1->esi->id, id);})))->esi;
			if (esiAMatar == NULL) {
				//si no lo encuentra acá, no existe el esi que se pretende matar.
				log_info(logger,
						"El ESI de id %s no se puede matar ya que no existe ningún ESI con ese id.",
						id);
			} else {
				EnviarDatosTipo(esiAMatar->socket, PLANIFICADOR, NULL, 0,
						ABORTAR);
				log_info(logger, "Abortando ESI por consola");
			}
		} else {
			EnviarDatosTipo(esiAMatar->socket, PLANIFICADOR, NULL, 0, ABORTAR);
			log_info(logger, "Abortando ESI por consola");
		}
	} else {
		EnviarDatosTipo(esiAMatar->socket, PLANIFICADOR, NULL, 0, ABORTAR);
		log_info(logger, "Abortando ESI por consola");
	}
}

void Status(char* clave) {
	clavexEsi* c =
			list_find(clavesBloqueadas,
					LAMBDA(
							bool _(clavexEsi* item1){return !strcmp(item1->clave,clave);}));
	if (c != NULL) {
		log_info(logger, "Estado de la clave \"%s\"", clave);
		if (strcmp(c->valor, "")) {
			log_info(logger, "\t\t\t valor: %s", c->valor);
			log_info(logger, "\t\t\t instancia actual: %s", c->instancia);
		} else {
			log_info(logger, "\t\t\t valor: Sin valor");
			log_info(logger, "\t\t\t potencial instancia: %s", c->instancia);
		}

		log_info(logger, "\t\t\t ESIs bloqueados por la clave: ");
		list_iterate(BLOQUEADOS, LAMBDA(void _(esiBloqueado* item1) {
			if (!strcmp(item1, clave)) {
				log_info(logger, "\t\t\t\t\t\t- %s: ", item1->esi->id)
				;
			}
		}
		));
	} else {
		log_info(logger,
				"No se puede mostrar el estado de la clave \"%s\" porque esta no existe.",
				clave);
	}
}

bool deadlockRecursivo(char* inicial, t_list* listaDL, esiBloqueado* esi,
		int* iteracion) {
	if (6 > iteracion[0]) {
		clavexEsi* cxe =
				list_find(clavesBloqueadas,
						LAMBDA(
								bool _(clavexEsi* item1){ return !strcmp(item1->clave, esi->clave);}));
		if (cxe != NULL) {
			char* idEsi = cxe->idEsi;
			esiBloqueado* bloqueado =
					list_find(BLOQUEADOS,
							LAMBDA(
									bool _(esiBloqueado* item1){ return !strcmp(item1->esi->id, idEsi);}));
			if (NULL != bloqueado) {
				if (!strcmp(bloqueado->clave, inicial)) {
					//hay deadlock
					t_list* deadlock = list_create();
					list_add(deadlock, bloqueado->esi->id);
					list_add(deadlock, esi->esi->id);
					list_add(listaDL, deadlock);
					return true;
				} else {
					//aplicar recursividad
					iteracion[0]++;
					deadlockRecursivo(inicial, listaDL, bloqueado, iteracion);
				}
			}
		}
	}
	return false;
}

void Deadlock() {
	int i;
	t_list* deadlocks = list_create();
	for (i = 0; i < list_size(BLOQUEADOS); i++) {
		esiBloqueado* bloqueado = list_get(BLOQUEADOS, i);
		t_list* claves =
				list_filter(clavesBloqueadas,
						LAMBDA(
								bool _(clavexEsi* item1){ return !strcmp(item1->idEsi, bloqueado->esi->id);}));
		int j;
		for (j = 0; j < list_size(claves); j++) {
			char* claveInicial = ((clavexEsi*) list_get(claves, j))->clave;
			int* iteracion = malloc(sizeof(int));
			iteracion[0] = 0;
			deadlockRecursivo(claveInicial, deadlocks, bloqueado, iteracion);
			free(iteracion);
		}

	}

	list_iterate(deadlocks,
			LAMBDA(
					void _(t_list* item1){ list_sort(item1, LAMBDA(bool _(char* i1, char*i2){ return i1 < i2; })); }));
	for (i = 0; i < list_size(deadlocks); i++) {
		t_list* deadlock = list_get(deadlocks, i);
		list_remove_by_condition(deadlocks,
				LAMBDA(
						bool _(t_list* item1){ if (list_size(item1) == list_size(deadlock)) { bool rv = true; int k=0; list_iterate(item1,LAMBDA(void _(char* idDelEsi){ if(rv) if (strcmp(idDelEsi, list_get(deadlock,k))) rv = false; k++; })); return rv; } else return false; }));
	}
	for (i = 0; i < list_size(deadlocks); i++) {
		t_list* deadlock = list_get(deadlocks, i);
		log_info(logger, "Deadlock %d entre ESIs:\n", (i + 1));
		list_iterate(deadlock, LAMBDA(void _(char* id) {
			log_info(logger, "\t\t\t- %s", id)
			;
		}
		));
	}
}

void consola() {
	char * linea;
	while (true) {
		linea = readline(">> ");
		if (linea)
			add_history(linea);
		if (!strcmp(linea, "pausar")) {
			if (!planificacion_detenida)
				PausarContinuar();
			else
				log_info(logger,
						"La planificación ya se encontraba pausada.\n");
		} else if (!strcmp(linea, "reanudar")) {
			if (planificacion_detenida)
				PausarContinuar();
			else
				log_info(logger,
						"La planificación ya se encontraba reanudada.\n");
		} else if (!strncmp(linea, "bloquear ", 9)) {
			char **array_input = string_split(linea, " ");
			Bloquear(array_input[2], array_input[1]);
			string_iterate_lines(array_input, free);
			free(array_input);
		} else if (!strncmp(linea, "desbloquear ", 12)) {
			char **array_input = string_split(linea, " ");
			Desbloquear(array_input[1], true);
			string_iterate_lines(array_input, free);
			free(array_input);
		} else if (!strncmp(linea, "listar ", 7)) {
			char **array_input = string_split(linea, " ");
			Listar(array_input[1]);
			string_iterate_lines(array_input, free);
			free(array_input);
		} else if (!strncmp(linea, "kill ", 5)) {
			char **array_input = string_split(linea, " ");
			Kill(array_input[1]);
			string_iterate_lines(array_input, free);
			free(array_input);
		} else if (!strncmp(linea, "status ", 7)) {
			char **array_input = string_split(linea, " ");
			Status(array_input[1]);
			string_iterate_lines(array_input, free);
			free(array_input);
		} else if (!strcmp(linea, "deadlock")) {
			printf("Se muestra deadlocks del sistema.\n");
			Deadlock();
		} else if (!strcmp(linea, "exit")) {
			printf("Salió\n");
			free(linea);
			break;
		} else
			printf("No se conoce el comando\n");
		free(linea);
	}
}
