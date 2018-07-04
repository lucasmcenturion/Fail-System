#include <sockets.h>
#define logearError(msg) {log_error(vg_logger,msg);}

/* Variables Globales */
char *IP, *ALGORITMO_DISTRIBUCION;
int PUERTO, CANT_ENTRADAS, TAMANIO_ENTRADA, RETARDO, socketPlanificador;
t_list* listaHilos;
bool end;
t_list* instancias;
t_list* esis;
t_list* instancias_caidas;
t_list* clavesNuevasPorEsi;
t_dictionary* clavesReemplazadas;
pthread_mutex_t mutex_instancias;
pthread_mutex_t mutex_esis;
pthread_mutex_t mutex_clavesNuevas;
t_log* vg_logger;

/*Creación de Logger*/
void crearLogger() {
	vg_logger = log_create("CoordinadorLog.log", "Coordinador", true,
			LOG_LEVEL_INFO);
}
//destroy logger
void finalizarLogger() {
	log_destroy(vg_logger);

}

char* simulacionGetProximoKE(int letraAscii){
	void* between(t_IdInstancia*aux){
			return letraAscii <= aux->finalKE ? true : false;
	}
	t_IdInstancia*rv= list_find(instancias, between);
	return rv->nombre;
}

char* simulacionGetProximo() {
	if (list_size(instancias) == 0)
		return "No hay instancias";
	t_list* instanciasClon = list_new();
	list_iterate(instancias,
					LAMBDA(
							void _(t_IdInstancia * elemento) {
		t_IdInstancia* inst = malloc(sizeof(t_IdInstancia));
		inst->socket = elemento->socket;
		inst->activo = elemento->activo;
		inst->nombre = malloc(strlen(elemento->nombre)+1);
		strcpy(inst->nombre, elemento->nombre);
		list_add(instanciasClon, inst);
	}));

	t_IdInstancia *aux;

	if (list_all_satisfy(instanciasClon,
			LAMBDA(
					bool _(t_IdInstancia * elemento) { return elemento->activo==true;}))) {
		list_iterate(instanciasClon,
				LAMBDA(
						void _(t_IdInstancia * elemento) { elemento->activo=false;}));
		aux = list_get(instanciasClon, 0);
		aux->activo = true;
		//list_replace(instancias, 0, aux);
		t_IdInstancia* inst = list_find(instancias, LAMBDA(bool _(t_IdInstancia * elemento) { return !strcmp(elemento->nombre,aux->nombre);}));
		list_destroy_and_destroy_elements(instanciasClon,LAMBDA(void _(t_IdInstancia * elemento) {free(elemento->nombre);free(elemento);}));
		return inst->nombre;
	}
	int i = -1;
	bool proximo(t_IdInstancia *elemento) {
		i++;
		if (!elemento->activo)
			return true;
	}
	list_find(instanciasClon, proximo);
	aux = list_get(instanciasClon, i);
	aux->activo = true;
	list_replace(instanciasClon, i, aux);
	t_IdInstancia* inst = list_find(instancias, LAMBDA(bool _(t_IdInstancia * elemento) { return !strcmp(elemento->nombre,aux->nombre);}));
	list_destroy_and_destroy_elements(instanciasClon,LAMBDA(void _(t_IdInstancia * elemento) {free(elemento->nombre);free(elemento);}));
	return inst->nombre;
}


int getProximoKE(int letraAscii){
	if (list_size(instancias) == 0)
		return 0;
	void* between(t_IdInstancia*aux){
			return letraAscii <= aux->finalKE ? true : false;
	}
	t_IdInstancia*rv= list_find(instancias, between);
	return rv->socket;
}
void reOrganizar(){
	int cant=list_size(instancias);
	if(cant==1){
		t_IdInstancia*aux = list_get(instancias,0);
		aux->inicialKE=97;
		aux->finalKE=122;
	}else{
		int cantidadDelPrimero = 26 / cant;
		int inicialResto = 96 + cantidadDelPrimero;
		int i=0;
		void setearLetras (t_IdInstancia *unaInstancia){
			if(i==0){
				unaInstancia->inicialKE=97;
				unaInstancia->finalKE = inicialResto;
				cantidadDelPrimero ++;
			}else{
				unaInstancia->inicialKE = inicialResto +1;
				if(cant-1 == i){
					unaInstancia->finalKE = 122;
				}else{
					unaInstancia->finalKE = unaInstancia->inicialKE + cantidadDelPrimero;
				}
				inicialResto = unaInstancia->finalKE;
			}
			i++;
		}
		list_iterate(instancias, setearLetras);
	}
}

void obteneryValidarValoresArchivoConfiguracion() {
	log_info(vg_logger, "Chequeo archivo de configuración");
	t_config* arch =
			config_create(
					"/home/utnso/workspace/tp-2018-1c-Fail-system/Coordinador/coordinador.cfg");
	if (config_has_property(arch, "IP") == true
			&& strlen(config_get_string_value(arch, "IP")) > 7) {
		IP = string_duplicate(config_get_string_value(arch, "IP"));
		log_info(vg_logger, "IP: %s", IP);
	} else {
		logearError("IP erroneo en Coordinador.cfg");
	}
	if (config_has_property(arch, "PUERTO") == true
			&& config_get_string_value(arch, "PUERTO") > 1) {
		PUERTO = config_get_int_value(arch, "PUERTO");
		log_info(vg_logger, "PUERTO: %d", PUERTO);
	} else {
		logearError("PUERTO erroneo en Coordinador.cfg");
	}
	ALGORITMO_DISTRIBUCION = string_duplicate(
			config_get_string_value(arch, "ALGORITMO_DISTRIBUCION"));
	CANT_ENTRADAS = config_get_int_value(arch, "CANT_ENTRADAS");
	TAMANIO_ENTRADA = config_get_int_value(arch, "TAMANIO_ENTRADA");
	RETARDO = config_get_int_value(arch, "RETARDO");
	config_destroy(arch);

}

void imprimirArchivoConfiguracion() {

	printf("Configuración:\n"
			"IP=%s\n"
			"PUERTO=%d\n"
			"ALGORITMO_DISTRIBUCION=%s\n"
			"CANT_ENTRADAS=%d\n"
			"TAMANIO_ENTRADA=%d\n"
			"RETARDO=%d\n", IP, PUERTO, ALGORITMO_DISTRIBUCION, CANT_ENTRADAS,
			TAMANIO_ENTRADA, RETARDO);
	fflush(stdout);
}

int comprobarValoresBienSeteados() {

	int retorno = 1;

	if (PUERTO != 8000) {
		logearError(
				"Archivo config de Coordinador: el puerto del COORDINADOR está mal configurado.");
		retorno = 0;
	}
	log_info(vg_logger, "Puerto Correcto");
	return retorno;
}
int getProximo() {
	if (list_size(instancias) == 0)
		return 0;
	t_IdInstancia *aux;

	if (list_all_satisfy(instancias,
			LAMBDA(
					bool _(t_IdInstancia * elemento) { return elemento->activo==true;}))) {
		list_iterate(instancias,
				LAMBDA(
						void _(t_IdInstancia * elemento) { elemento->activo=false;}));
		aux = list_get(instancias, 0);
		aux->activo = true;
		//list_replace(instancias, 0, aux);
		return aux->socket;
	}
	int i = -1;
	bool proximo(t_IdInstancia *elemento) {
		i++;
		if (!elemento->activo)
			return true;
	}
	list_find(instancias, proximo);
	aux = list_get(instancias, i);
	aux->activo = true;
	list_replace(instancias, i, aux);
	return aux->socket;
}

void sacar_instancia(int socket) {
	bool tiene_socket(t_IdInstancia *instancia) {
		if (instancia->socket == socket)
			return true;
	}
	t_IdInstancia*remove = list_remove_by_condition(instancias,
			(void*) tiene_socket);
	if (remove) {
		list_add(instancias_caidas, remove);
	} else {
		//termino un ESI
		bool tiene_socket_esi(t_esiCoordinador *esi) {
			if (esi->socket == socket)
				return true;
		}
		list_remove_by_condition(esis, tiene_socket_esi);
	}
}

bool verificarGet(char *idEsi, char* keyEsi) {
	t_esiCoordinador *aux = list_find(esis,
			LAMBDA(int _(t_esiCoordinador *elemento) {
				return !strcmp(elemento->id, idEsi);
			}
	));
	return list_any_satisfy(aux->claves, LAMBDA(int _(char *elemento) {
		return !strcmp(elemento, keyEsi);
	}
	));
}
int obtenerSocket(char* keyEsi) {
	bool compararClaves(t_IdInstancia*elemento) {
		return list_any_satisfy(elemento->claves, LAMBDA(int _(char *e) {
			return !strcmp(e, keyEsi);
		}
		));
	}
	t_IdInstancia *aux = list_find(instancias, compararClaves);
	return aux ? aux->socket : 0;
}
char* obtenerId(char* key, int flag) {
	bool encontrarClave(t_esiCoordinador *elemento) {
		return list_any_satisfy(elemento->claves, LAMBDA(int _(char *e) {
			return !strcmp(e, key);
		}
		));
	}
	pthread_mutex_lock(&mutex_esis);
	t_esiCoordinador*aux = list_find(esis, encontrarClave);
	if (flag == 1)
		list_remove_by_condition(aux->claves, LAMBDA(int _(char *elem) {
			return !strcmp(elem, key);
		}
		));
	pthread_mutex_unlock(&mutex_esis);
	return aux->id;
}
bool esInstanciaCaida(char* nombreInstancia) {
	return list_any_satisfy(instancias_caidas, LAMBDA(int _(t_IdInstancia *e) {
		return !strcmp(e->nombre, nombreInstancia);
	}
	));
}
void accion(void* socket) {
	int socketFD = *(int*) socket;
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteServidorCoordinador(socketFD, COORDINADOR, &paquete)
			> 0) {
		//SWITCH
		if (!strcmp(paquete.header.emisor, INSTANCIA)) {
			switch (paquete.header.tipoMensaje) {
			case ESHANDSHAKE: {
				EnviarDatosTipo(socketFD, COORDINADOR, 1, 0, SOLICITUDNOMBRE);
				int tamanioDatosEntradas = sizeof(int) * 2;
				void *datosEntradas = malloc(tamanioDatosEntradas);
				*((int*) datosEntradas) = TAMANIO_ENTRADA;
				datosEntradas += sizeof(int);
				*((int*) datosEntradas) = CANT_ENTRADAS;
				datosEntradas += sizeof(int);
				datosEntradas -= tamanioDatosEntradas;
				EnviarDatosTipo(socketFD, COORDINADOR, datosEntradas,
						tamanioDatosEntradas, GETENTRADAS);
				free(datosEntradas);
			}
				break;
			case IDENTIFICACIONINSTANCIA: {
				char *nombreInstancia = malloc(paquete.header.tamPayload);
				strcpy(nombreInstancia, (char*) paquete.Payload);
				t_IdInstancia *instancia;
				if (esInstanciaCaida(nombreInstancia)) {
					bool mismoId(t_IdInstancia*e) {
						return !strcmp(e->nombre, nombreInstancia) ?
								true : false;
					}
					instancia = list_remove_by_condition(instancias_caidas,
							mismoId);
				} else {
					instancia = malloc(sizeof(t_IdInstancia));
					instancia->nombre = nombreInstancia;
					instancia->claves = list_create();
				}
				instancia->socket = socketFD;
				instancia->activo = false;
				pthread_mutex_lock(&mutex_instancias);
				list_add(instancias, instancia);
				if(!strcmp(ALGORITMO_DISTRIBUCION,"KE"))
					reOrganizar();
				pthread_mutex_unlock(&mutex_instancias);
			}
				break;
			case SETOK: {
				int tiene_socket(t_IdInstancia *e) {
					return e->socket == socketFD;
				}
				char* claveNueva = malloc(strlen((char*) paquete.Payload) + 1);
				strcpy(claveNueva, paquete.Payload);
				char* value = malloc(strlen(paquete.Payload+strlen(claveNueva)+1)+1);
				strcpy(value,paquete.Payload+strlen(claveNueva)+1);
				pthread_mutex_lock(&mutex_instancias);
				t_IdInstancia *aux = list_find(instancias, tiene_socket);
				list_add(aux->claves, claveNueva);
				pthread_mutex_unlock(&mutex_instancias);
				char *idEsi = malloc(10);
				strcpy(idEsi, obtenerId((char*) paquete.Payload, 0));
				idEsi = realloc(idEsi, strlen(idEsi) + 1);
				log_info(vg_logger, "Se hizo OK el SET");
				void* idvalueykey = malloc(strlen(idEsi)+strlen(value)+ strlen(claveNueva) + strlen(aux->nombre) + 4);
				strcpy(idvalueykey, idEsi);
				strcpy(idvalueykey + strlen(idEsi) + 1, value);
				strcpy(idvalueykey + strlen(idEsi) + strlen(value) + 2, claveNueva);
				EnviarDatosTipo(socketPlanificador, COORDINADOR, idvalueykey,
						strlen(idEsi) + strlen(value) + strlen(claveNueva) + strlen(aux->nombre) + 4, SETOKPLANI);
				free(idvalueykey);
				free(value);
				free(idEsi);
			}
				break;
			case STOREOK: {
				log_info(vg_logger, "Se hizo OK el STORE");
				EnviarDatosTipo(socketPlanificador, COORDINADOR,
						paquete.Payload, paquete.header.tamPayload,
						STOREOKPLANI);
			}
				break;
			case ELIMINARCLAVE: {
				int tiene_socket(t_IdInstancia *e) {
					return e->socket == socketFD;
				}
				pthread_mutex_lock(&mutex_instancias);
				t_IdInstancia *aux = list_find(instancias, tiene_socket);
				list_remove_by_condition(aux->claves,
						LAMBDA(int _(char *clave) {
							return !strcmp(clave, (char* )paquete.Payload);
						}
				));
				pthread_mutex_unlock(&mutex_instancias);
				char*claveReemplazada = malloc(strlen(paquete.Payload) + 1);
				strcpy(claveReemplazada, paquete.Payload);
				dictionary_put(clavesReemplazadas, claveReemplazada, true);
			}
				break;
			}
		}
		if (!strcmp(paquete.header.emisor, ESI)) {
			switch (paquete.header.tipoMensaje) {
			case ESHANDSHAKE: {
				t_esiCoordinador*nuevo = malloc(sizeof(t_esiCoordinador));
				nuevo->id = malloc(paquete.header.tamPayload);
				strcpy(nuevo->id, paquete.Payload);
				nuevo->socket = socketFD;
				nuevo->claves = list_create();
				pthread_mutex_lock(&mutex_esis);
				list_add(esis, nuevo);
				pthread_mutex_unlock(&mutex_esis);
			}
				break;
			case SETCOORD: {
				usleep(RETARDO);
				datos = paquete.Payload;
				char*key = malloc(strlen(datos) + 1);
				strcpy(key, datos);
				datos += strlen(datos) + 1;
				char* value = malloc(strlen(datos) + 1);
				strcpy(value, datos);

				bool verificarClave(t_IdInstancia *e) {
					bool rv = list_any_satisfy(e->claves,
							LAMBDA(int _(char *clave) {
								return !strcmp(clave, key);
							}
					));
					return rv;
				}

					char*id=malloc(10);
					strcpy(id,((t_esiCoordinador*)list_find(esis,LAMBDA(int _(t_esiCoordinador *elemento) {  return elemento->socket ==socketFD;})))->id);
					id=realloc(id,strlen(id)+1);
					log_info(vg_logger, "El COORDINADOR recibió operación SET del ESI: %s, con clave: %s y valor: %s \n", id, key, value);
				if (strlen(key) > 40) {
					log_info(vg_logger,
							"Se quiere hacer un SET de una clave que excede los 40 caracteres");
					EnviarDatosTipo(socketPlanificador, COORDINADOR, id,
							strlen(id) + 1, ABORTAR);
				} else {
					if (verificarGet(id, key)) {
						bool verificarNuevaPorEsi(clavexEsi* e) {
							return !strcmp(id, e->idEsi) && !strcmp(key, e->clave);
						}

						pthread_mutex_lock(&mutex_clavesNuevas);
						if (list_find((t_list*) clavesNuevasPorEsi,
								(void*) verificarNuevaPorEsi)) {
							int tam = strlen(key) + strlen(value) + 2;
							void*sendInstancia = malloc(tam);
							strcpy(sendInstancia, key);
							sendInstancia += strlen(key) + 1;
							strcpy(sendInstancia, value);
							sendInstancia += strlen(value) + 1;
							sendInstancia -= tam;
							if (!strcmp(ALGORITMO_DISTRIBUCION, "EL")) {
								pthread_mutex_lock(&mutex_instancias);
								int socketSiguiente = getProximo();
								if (socketSiguiente != 0) {
									EnviarDatosTipo(socketSiguiente,
											COORDINADOR, sendInstancia, tam,
											SETINST);
								} else {
									//error, no hay instancias conectadas al sistema
									log_info(vg_logger,
											"No hay instancias conectadas al sistema");
									fflush(stdout);
								}
								pthread_mutex_unlock(&mutex_instancias);
							}else {
								if(!strcmp(ALGORITMO_DISTRIBUCION,"KE")){
									pthread_mutex_lock(&mutex_instancias);
									int socketSiguiente = getProximoKE((int)key[0]);
									if(socketSiguiente!=0){
										EnviarDatosTipo(socketSiguiente,COORDINADOR,sendInstancia,tam,SETINST);
									}else{
										//error, no hay instancias conectadas al sistema
										log_info(vg_logger,"No hay instancias conectadas al sistema");
										fflush(stdout);
									}
									pthread_mutex_unlock(&mutex_instancias);
								}
							}
						} else {
							pthread_mutex_lock(&mutex_instancias);
							t_IdInstancia *aux = list_find(instancias,
									verificarClave);
							pthread_mutex_unlock(&mutex_instancias);
							if (!aux) {
								//clave existe en el sistema, pero la instancia esta caida
								log_info(vg_logger,
										"Se intenta bloquear la clave %s pero en este momento no esta disponible",
										key);
								fflush(stdout);
								EnviarDatosTipo(socketPlanificador, COORDINADOR,
										id, strlen(id) + 1, ABORTAR);
							} else {
								int tam = strlen(key) + strlen(value) + 2;
								void*sendInstancia = malloc(tam);
								strcpy(sendInstancia, key);
								sendInstancia += strlen(key) + 1;
								strcpy(sendInstancia, value);
								sendInstancia += strlen(value) + 1;
								sendInstancia -= tam;
								EnviarDatosTipo(aux->socket, COORDINADOR,
										sendInstancia, tam, SETINST);
							}
						}
						if (!list_is_empty(clavesNuevasPorEsi)) {
							clavexEsi*aEliminar = list_remove_by_condition(
									(t_list*) clavesNuevasPorEsi,
									(void*) verificarNuevaPorEsi);
							if (aEliminar) {
								free(aEliminar->idEsi);
								free(aEliminar->clave);
								free(aEliminar);
							}
						}
						pthread_mutex_unlock(&mutex_clavesNuevas);
					} else {
						log_info(vg_logger,
								"Se intenta hacer un SET de la clave %s pero nunca se hizo un GET",
								key);
						EnviarDatosTipo(socketPlanificador, COORDINADOR, id,
								strlen(id) + 1, ABORTAR);
					}
				}
				free(key);
				free(value);
				free(id);
			}
				break;
			case GETCOORD: {
				usleep(RETARDO);
				char* lakey = malloc(strlen(paquete.Payload) + 1);
				strcpy(lakey, (char*) paquete.Payload);
				pthread_mutex_lock(&mutex_esis);
				t_esiCoordinador *aux = list_find(esis,
						LAMBDA(int _(t_esiCoordinador *elemento) {
							return elemento->socket == socketFD;
						}
				));
				if (!list_any_satisfy(aux->claves, LAMBDA(int _(char*elemento) {
					return !strcmp(elemento, lakey);
				}
				))) {
					clavexEsi* nueva = malloc(sizeof(clavexEsi));
					nueva->clave = malloc(strlen(lakey) + 1);
					strcpy(nueva->clave, lakey);
					char* id = malloc(strlen(aux->id) + 1);
					strcpy(id, (aux->id));
					nueva->idEsi = malloc(strlen(id));
					strcpy(nueva->idEsi, id);
					free(id);
					list_add(aux->claves, lakey);
					pthread_mutex_lock(&mutex_clavesNuevas);
					list_add(clavesNuevasPorEsi, nueva);
					pthread_mutex_unlock(&mutex_clavesNuevas);
				}
				pthread_mutex_unlock(&mutex_esis);
				char*id = malloc(10);
				strcpy(id, aux->id);
				id = realloc(id, strlen(id) + 1);
				if (strlen((char*) paquete.Payload) > 40) {
					printf(
							"Se intenta hacer un GET de una clave que excede el tamaño maximo\n");
					EnviarDatosTipo(socketPlanificador, COORDINADOR, id,
							strlen(id) + 1, ABORTAR);
				} else {
					char* instancia;
					if (!strcmp(ALGORITMO_DISTRIBUCION, "EL")){
						instancia = simulacionGetProximo();
					}
					else if (!strcmp(ALGORITMO_DISTRIBUCION, "KE")){
						instancia = simulacionGetProximoKE((int)lakey[0]);
					}
					int tamSend = strlen(paquete.Payload) + strlen(id) + strlen(instancia) + 3;
					void* sendPlanificador = malloc(tamSend);
					strcpy(sendPlanificador, paquete.Payload);
					sendPlanificador += strlen(paquete.Payload) + 1;
					strcpy(sendPlanificador, id);
					sendPlanificador += strlen(id) + 1;
					strcpy(sendPlanificador, instancia);
					sendPlanificador += strlen(instancia) + 1;
					sendPlanificador -= tamSend;
					EnviarDatosTipo(socketPlanificador, COORDINADOR,
							sendPlanificador, tamSend, GETPLANI);

					free(sendPlanificador);
				}
				log_info(vg_logger,
						"El COORDINADOR recibió operación GET del ESI: %s, con clave: %s\n",
						id, lakey);
				free(id);

			}
				break;
			case STORECOORD: {
				usleep(RETARDO);
				char*id = malloc(10);
				strcpy(id,
						((t_esiCoordinador*) list_find(esis,
								LAMBDA(int _(t_esiCoordinador *elemento) {
									return elemento->socket == socketFD;
								}
						)))->id);
				id = realloc(id, strlen(id) + 1);
				if (strlen((char*) paquete.Payload) > 40) {
					printf(
							"Se quiere hacer un SET de una clave que excede los 40 caracteres\n");
					EnviarDatosTipo(socketPlanificador, COORDINADOR, id,
							strlen(id) + 1, ABORTAR);
				} else {
					log_info(vg_logger,
							"El COORDINADOR recibió operación STORE del ESI: %s, con clave: %s\n",
							id, paquete.Payload);
					if (verificarGet(id, (char*) paquete.Payload)) {
						int socketInstancia = obtenerSocket(paquete.Payload);
						if (socketInstancia != 0)
							EnviarDatosTipo(socketInstancia, COORDINADOR,
									(char*) paquete.Payload,
									strlen((char*) paquete.Payload) + 1,
									STOREINST);
						else {
							if (dictionary_has_key(clavesReemplazadas,
									(char*) paquete.Payload)) {
								log_info(vg_logger,
										"Se intenta hacer un STORE de la clave %s pero fue reemplazada",
										(char*) paquete.Payload);
							} else {
								log_info(vg_logger,
										"Se intenta hacer un STORE de la clave %s pero no se hizo un GET",
										(char*) paquete.Payload);
							}
							EnviarDatosTipo(socketPlanificador, COORDINADOR, id,
									strlen(id) + 1, ABORTAR);
						}
					} else {
						if (dictionary_has_key(clavesReemplazadas,
								(char*) paquete.Payload)) {
							log_info(vg_logger,
									"Se intenta hacer un STORE de la clave %s pero fue reemplazada",
									(char*) paquete.Payload);
						} else {
							log_info(vg_logger,
									"Se intenta hacer un STORE de la clave %s pero no se hizo un GET",
									(char*) paquete.Payload);
						}
						EnviarDatosTipo(socketPlanificador, COORDINADOR, id,
								strlen(id) + 1, ABORTAR);
					}
				}
				free(id);
				//log_info(vg_logger, "El COORDINADOR recibe operación STORE del ESI: %s"....);
			}
				break;
			}
		}
		if (!strcmp(paquete.header.emisor, PLANIFICADOR)) {
			switch (paquete.header.tipoMensaje) {
			case ESHANDSHAKE: {
				socketPlanificador = socketFD;
				datos = paquete.Payload;
				int tope = ((uint32_t*) datos)[0];
				datos += sizeof(uint32_t);
				for (int i = 0; i < tope; i++) {
					printf("%s\n", datos);
					datos += strlen(datos) + 1;
				}
			}
				break;
			}
		}
		if (paquete.Payload != NULL)
			free(paquete.Payload);
	}
	close(socketFD);
	pthread_mutex_lock(&mutex_instancias);
	sacar_instancia(socketFD);
	pthread_mutex_unlock(&mutex_instancias);
}

int main(void) {
	crearLogger();
	obteneryValidarValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
	pthread_mutex_init(&mutex_instancias, NULL);
	pthread_mutex_init(&mutex_esis, NULL);
	pthread_mutex_init(&mutex_clavesNuevas, NULL);
	esis = list_create();
	instancias = list_create();
	instancias_caidas = list_create();
	clavesNuevasPorEsi = list_create();
	clavesReemplazadas = dictionary_create();
	ServidorConcurrente(IP, PUERTO, COORDINADOR, &listaHilos, &end, accion);
	finalizarLogger();
	return EXIT_SUCCESS;
}
