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
t_dictionary* clavesNuevas;
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
	t_IdInstancia*remove = list_remove_by_condition(instancias, (void*)tiene_socket);
	if(remove){
		list_add(instancias_caidas,remove);
	}else{
		//termino un ESI
		bool tiene_socket_esi(t_esiCoordinador *esi) {
				if (esi->socket == socket)
					return true;
		}
		list_remove_by_condition(esis,tiene_socket_esi);
	}
}

bool verificarGet(char *idEsi, char* keyEsi){
	t_esiCoordinador *aux = list_find(esis, LAMBDA(int _(t_esiCoordinador *elemento) {  return !strcmp(elemento->id,idEsi);}));
	return list_any_satisfy(aux->claves,LAMBDA(int _(char *elemento) {  return !strcmp(elemento,keyEsi);}));
}
int obtenerSocket(char* keyEsi){
	bool compararClaves(t_IdInstancia*elemento){
		return list_any_satisfy(elemento->claves,LAMBDA(int _(char *e) {  return !strcmp(e,keyEsi);}));
	}
	t_IdInstancia *aux = list_find(instancias, compararClaves);
	return aux ? aux->socket : 0;
}
char* obtenerId(char* key,int flag){
	bool encontrarClave(t_esiCoordinador *elemento){
		return list_any_satisfy(elemento->claves,LAMBDA(int _(char *e) {  return !strcmp(e,key);}));
	}
	pthread_mutex_lock(&mutex_esis);
	t_esiCoordinador*aux=list_find(esis,encontrarClave);
	if(flag==1)
		list_remove_by_condition(aux->claves,LAMBDA(int _(char *elem) {  return !strcmp(elem,key);}));
	pthread_mutex_unlock(&mutex_esis);
	return aux->id;
}
bool esInstanciaCaida(char* nombreInstancia){
	return list_any_satisfy(instancias_caidas,LAMBDA(int _(t_IdInstancia *e) {  return !strcmp(e->nombre,nombreInstancia);}));
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
				EnviarDatosTipo(socketFD, COORDINADOR, 1,0, SOLICITUDNOMBRE);
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
				if(esInstanciaCaida(nombreInstancia)){
					bool mismoId(t_IdInstancia*e){
						return !strcmp(e->nombre,nombreInstancia) ? true : false;
					}
					instancia=list_remove_by_condition(instancias_caidas, mismoId);
				}else{
					instancia = malloc(sizeof(t_IdInstancia));
					instancia->nombre = nombreInstancia;
					instancia->claves = list_create();
				}
				instancia->socket = socketFD;
				instancia->activo = false;
				pthread_mutex_lock(&mutex_instancias);
				list_add(instancias, instancia);
				pthread_mutex_unlock(&mutex_instancias);
			}
			break;
			case SETOK: {
				int tiene_socket(t_IdInstancia *e) {
						return e->socket == socketFD;
				}
				char* claveNueva = malloc(strlen((char*)paquete.Payload)+1);
				strcpy(claveNueva,paquete.Payload);
				pthread_mutex_lock(&mutex_instancias);
				t_IdInstancia *aux = list_find(instancias, tiene_socket);
				list_add(aux->claves,claveNueva);
				pthread_mutex_unlock(&mutex_instancias);
				char *idEsi = malloc(10);
				strcpy(idEsi,obtenerId((char*)paquete.Payload,0));
				idEsi=realloc(idEsi,strlen(idEsi)+1);
				log_info(vg_logger, "Se hizo OK el SET");
				EnviarDatosTipo(socketPlanificador,COORDINADOR,idEsi,strlen(idEsi)+1,SETOKPLANI);
				free(idEsi);
			}
			break;
			case STOREOK: {
				log_info(vg_logger, "Se hizo OK el STORE");
				EnviarDatosTipo(socketPlanificador,COORDINADOR,paquete.Payload,paquete.header.tamPayload,STOREOKPLANI);
			}
			break;
			case ELIMINARCLAVE: {
				int tiene_socket(t_IdInstancia *e) {
						return e->socket == socketFD;
				}
				pthread_mutex_lock(&mutex_instancias);
				t_IdInstancia *aux = list_find(instancias, tiene_socket);
				list_remove_by_condition(aux->claves,LAMBDA(int _(char *clave) {  return !strcmp(clave,(char*)paquete.Payload);}));
				pthread_mutex_unlock(&mutex_instancias);
				char*claveReemplazada=malloc(strlen(paquete.Payload)+1);
				strcpy(claveReemplazada,paquete.Payload);
				dictionary_put(clavesReemplazadas,claveReemplazada,true);
			}
			break;
			}
		}
		if(!strcmp(paquete.header.emisor, ESI)){
			switch(paquete.header.tipoMensaje){
				case ESHANDSHAKE:{
					t_esiCoordinador*nuevo =malloc(sizeof(t_esiCoordinador));
					nuevo->id=malloc(paquete.header.tamPayload);
					strcpy(nuevo->id,paquete.Payload);
					nuevo->socket = socketFD;
					nuevo->claves = list_create();
					pthread_mutex_lock(&mutex_esis);
					list_add(esis,nuevo);
					pthread_mutex_unlock(&mutex_esis);
				}
				break;
				case SETCOORD: {
					usleep(RETARDO);
					datos=paquete.Payload;
					char*key = malloc(strlen(datos) + 1);
					strcpy(key, datos);
					datos += strlen(datos) + 1;
					char* value = malloc(strlen(datos) + 1);
					strcpy(value, datos);

					bool verificarClave(t_IdInstancia *e){
						bool rv = list_any_satisfy(e->claves,LAMBDA(int _(char *clave) {  return !strcmp(clave,key);}));
						return rv;
					}

					char*id=malloc(10);
					strcpy(id,((t_esiCoordinador*)list_find(esis,LAMBDA(int _(t_esiCoordinador *elemento) {  return elemento->socket ==socketFD;})))->id);
					id=realloc(id,strlen(id)+1);
					log_info(vg_logger, "El COORDINADOR recibió operación SET del ESI: %s, con clave: %s y valor: %s \n", id, key, value);
					if(strlen(key)>40){
						log_info(vg_logger,"Se quiere hacer un SET de una clave que excede los 40 caracteres");
						EnviarDatosTipo(socketPlanificador,COORDINADOR,id,strlen(id)+1,ABORTAR);
					}else{
						if(verificarGet(id,key)){
							pthread_mutex_lock(&mutex_clavesNuevas);
							if(dictionary_has_key(clavesNuevas,key)){
								int tam=strlen(key)+strlen(value)+2;
								void*sendInstancia = malloc(tam);
								strcpy(sendInstancia,key);
								sendInstancia+=strlen(key)+1;
								strcpy(sendInstancia,value);
								sendInstancia+=strlen(value)+1;
								sendInstancia-=tam;
								if(!strcmp(ALGORITMO_DISTRIBUCION,"EL")){
									pthread_mutex_lock(&mutex_instancias);
									int socketSiguiente = getProximo();
									if(socketSiguiente!=0){
										EnviarDatosTipo(socketSiguiente,COORDINADOR,sendInstancia,tam,SETINST);
									}else{
										//error, no hay instancias conectadas al sistema
										log_info(vg_logger,"No hay instancias conectadas al sistema");
										fflush(stdout);
									}
									pthread_mutex_unlock(&mutex_instancias);
								}
							}else{
								pthread_mutex_lock(&mutex_instancias);
								t_IdInstancia *aux=list_find(instancias,verificarClave);
								pthread_mutex_unlock(&mutex_instancias);
								if(!aux){
									//clave existe en el sistema, pero la instancia esta caida
									log_info(vg_logger,"Se intenta bloquear la clave %s pero en este momento no esta disponible",key);
									fflush(stdout);
									EnviarDatosTipo(socketPlanificador,COORDINADOR,id,strlen(id)+1,ABORTAR);
								}else{
									int tam=strlen(key)+strlen(value)+2;
									void*sendInstancia = malloc(tam);
									strcpy(sendInstancia,key);
									sendInstancia+=strlen(key)+1;
									strcpy(sendInstancia,value);
									sendInstancia+=strlen(value)+1;
									sendInstancia-=tam;
									EnviarDatosTipo(aux->socket,COORDINADOR,sendInstancia,tam,SETINST);
								}
							}
							dictionary_remove(clavesNuevas,key);
							pthread_mutex_unlock(&mutex_clavesNuevas);
						}else{
							log_info(vg_logger,"Se intenta hacer un SET de la clave %s pero nunca se hizo un GET",key);
							EnviarDatosTipo(socketPlanificador,COORDINADOR,id,strlen(id)+1,ABORTAR);
						}
					}
					free(key);
					free(value);
					free(id);
			}
			break;
			case GETCOORD: {
				usleep(RETARDO);
				char* lakey = malloc(strlen(paquete.Payload)+1);
				strcpy(lakey,(char*)paquete.Payload);
				pthread_mutex_lock(&mutex_esis);
				t_esiCoordinador *aux = list_find(esis,LAMBDA(int _(t_esiCoordinador *elemento) {  return elemento->socket ==socketFD;}));
				if(!list_any_satisfy(aux->claves,LAMBDA(int _(char*elemento) {  return !strcmp(elemento,lakey);}))){
					list_add(aux->claves,lakey);
					pthread_mutex_lock(&mutex_clavesNuevas);
					dictionary_put(clavesNuevas,(char*)paquete.Payload,true);
					pthread_mutex_unlock(&mutex_clavesNuevas);
				}
				pthread_mutex_unlock(&mutex_esis);
				char*id=malloc(10);
				strcpy(id,aux->id);
				id=realloc(id,strlen(id)+1);
				if(strlen((char*)paquete.Payload) > 40){
					printf("Se intenta hacer un GET de una clave que excede el tamaño maximo\n");
					EnviarDatosTipo(socketPlanificador,COORDINADOR,id,strlen(id)+1,ABORTAR);
				}else{
					int tamSend=strlen(paquete.Payload)+strlen(id)+2;
					void* sendPlanificador = malloc(tamSend);
					strcpy(sendPlanificador,paquete.Payload);
					sendPlanificador+=strlen(paquete.Payload)+1;
					strcpy(sendPlanificador,id);
					sendPlanificador+=strlen(id)+1;
					sendPlanificador-=tamSend;
					EnviarDatosTipo(socketPlanificador,COORDINADOR,sendPlanificador,tamSend,GETPLANI);

					free(sendPlanificador);
				}
				log_info(vg_logger, "El COORDINADOR recibió operación GET del ESI: %s, con clave: %s\n", id, lakey);
				free(id);

			}
			break;
			case STORECOORD: {
				usleep(RETARDO);
				char*id=malloc(10);
				strcpy(id,((t_esiCoordinador*)list_find(esis,LAMBDA(int _(t_esiCoordinador *elemento) {  return elemento->socket ==socketFD;})))->id);
				id=realloc(id,strlen(id)+1);
				if(strlen((char*)paquete.Payload)>40){
					printf("Se quiere hacer un SET de una clave que excede los 40 caracteres\n");
					EnviarDatosTipo(socketPlanificador,COORDINADOR,id,strlen(id)+1,ABORTAR);
				}else{
					log_info(vg_logger, "El COORDINADOR recibió operación STORE del ESI: %s, con clave: %s\n", id, paquete.Payload);
					if(verificarGet(id,(char*)paquete.Payload)){
						int socketInstancia = obtenerSocket(paquete.Payload);
						if(socketInstancia!=0)
							EnviarDatosTipo(socketInstancia,COORDINADOR,(char*)paquete.Payload,strlen((char*)paquete.Payload)+1,STOREINST);
						else{
							if(dictionary_has_key(clavesReemplazadas,(char*)paquete.Payload)){
								log_info(vg_logger,"Se intenta hacer un STORE de la clave %s pero fue reemplazada",(char*)paquete.Payload);
							}else{
								log_info(vg_logger,"Se intenta hacer un STORE de la clave %s pero no se hizo un GET",(char*)paquete.Payload);
							}
							EnviarDatosTipo(socketPlanificador,COORDINADOR,id,strlen(id)+1,ABORTAR);
						}
					}else{
						if(dictionary_has_key(clavesReemplazadas,(char*)paquete.Payload)){
							log_info(vg_logger,"Se intenta hacer un STORE de la clave %s pero fue reemplazada",(char*)paquete.Payload);
						}else{
							log_info(vg_logger,"Se intenta hacer un STORE de la clave %s pero no se hizo un GET",(char*)paquete.Payload);
						}
						EnviarDatosTipo(socketPlanificador,COORDINADOR,id,strlen(id)+1,ABORTAR);
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
				int tope = ((uint32_t*)datos)[0];
				datos += sizeof(uint32_t);
				for (int i=0; i < tope; i++){
					printf("%s\n", datos);
					datos += strlen(datos) +1;
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
	pthread_mutex_init(&mutex_instancias,NULL);
	pthread_mutex_init(&mutex_esis,NULL);
	pthread_mutex_init(&mutex_clavesNuevas,NULL);
	esis=list_create();
	instancias=list_create();
	instancias_caidas=list_create();
	clavesNuevas = dictionary_create();
	clavesReemplazadas = dictionary_create();
	ServidorConcurrente(IP, PUERTO, COORDINADOR, &listaHilos, &end, accion);
	finalizarLogger();
	return EXIT_SUCCESS;
}
