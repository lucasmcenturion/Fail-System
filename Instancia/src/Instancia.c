#include "sockets.h"
#include <dirent.h>
#include <commons/collections/list.h>

char *IP_COORDINADOR, *ALGORITMO_REEMPLAZO, *PUNTO_MONTAJE, *NOMBRE_INSTANCIA;
int PUERTO_COORDINADOR, INTERVALO_DUMP, TAMANIO_ENTRADA, CANT_ENTRADA,
		ENTRADAS_LIBRES, socketCoordinador;
char **tabla_entradas;
t_list *entradas_administrativa;
t_log * logger;
t_dictionary *aux_compactacion;
pthread_mutex_t mutex_entradas;

/*Creación de Logger*/
void crearLogger() {
	logger = log_create("InstanciaLog.log", "INSTANCIA", true, LOG_LEVEL_INFO);
}

void obtenerValoresArchivoConfiguracion(char* id) {
	t_config* arch =
			config_create(
					"/home/utnso/workspace/tp-2018-1c-Fail-system/Instancia/instancia.cfg");
	IP_COORDINADOR = string_duplicate(
			config_get_string_value(arch, "IP_COORDINADOR"));
	PUERTO_COORDINADOR = config_get_int_value(arch, "PUERTO_COORDINADOR");
	ALGORITMO_REEMPLAZO = string_duplicate(
			config_get_string_value(arch, "ALGORITMO_REEMPLAZO"));
	PUNTO_MONTAJE = string_duplicate(
			config_get_string_value(arch, "PUNTO_MONTAJE"));
	NOMBRE_INSTANCIA = string_duplicate(
			config_get_string_value(arch, "NOMBRE_INSTANCIA"));
	PUNTO_MONTAJE = string_from_format("/home/utnso/inst%s", id);
	NOMBRE_INSTANCIA = string_from_format("Inst%s", id);
	INTERVALO_DUMP = config_get_int_value(arch, "INTERVALO_DUMP");
	config_destroy(arch);
}

void imprimirArchivoConfiguracion() {
	printf("Configuración:\n"
			"IP_COORDINADOR=%s\n"
			"PUERTO_COORDINADOR=%d\n"
			"ALGORITMO_REEMPLAZO=%s\n"
			"PUNTO_MONTAJE=%s\n"
			"NOMBRE_INSTANCIA=%s\n"
			"INTERVALO_DUMP=%d\n", IP_COORDINADOR, PUERTO_COORDINADOR,
			ALGORITMO_REEMPLAZO, PUNTO_MONTAJE, NOMBRE_INSTANCIA,
			INTERVALO_DUMP);
	fflush(stdout);
}
void compactacion(bool condicion) {
	/*Voy a obtener todos los valores asociados, los pongo en un dictionary
	 * y luego voy copiando uno por uno de nuevo al array*/

	void insertarEnDictionary(t_Entrada * aux){
		//obtenemos el valor asociado
		char *valueReturn = malloc(aux->tamanio + 1);
		for (int var = aux->index;var < aux->index + aux->entradasOcupadas; var++) {
			if ((aux->index + aux->entradasOcupadas) - 1 == var) {
				strcpy(valueReturn, tabla_entradas[var]);
				valueReturn += strlen(tabla_entradas[var]);
				break;
			}
			strncpy(valueReturn, tabla_entradas[var], TAMANIO_ENTRADA);
			valueReturn += TAMANIO_ENTRADA;
		}
		valueReturn -= aux->tamanio;
		dictionary_put(aux_compactacion,aux->clave,valueReturn);
		int i;
		for (i = aux->index; i < (aux->index + aux->entradasOcupadas);i++) {
			strcpy(tabla_entradas[i],"NaN");
		}

	}

	t_list*activos= list_filter(entradas_administrativa,LAMBDA(int _(t_Entrada *ent) {return ent->activo;}));
	list_iterate(activos, insertarEnDictionary);

	void reOrdenarEntradas(t_Entrada *e){
		//nuevo index
		e->index = getFirstIndex(e->entradasOcupadas);
		char *valueAux = malloc(strlen(dictionary_get(aux_compactacion,e->clave)) + 1);
		strcpy(valueAux, dictionary_get(aux_compactacion,e->clave));
		int j;
		//copio en tabla entradas posta
		for (j = e->index; j < (e->index + e->entradasOcupadas);j++) {
			if ((e->index + e->entradasOcupadas) - 1 == j) {
				strcpy(tabla_entradas[j], valueAux);
				break;
			}
			strncpy(tabla_entradas[j], valueAux, TAMANIO_ENTRADA);
			valueAux += TAMANIO_ENTRADA;
		}
		free(valueAux);
	}
	list_iterate(activos, reOrdenarEntradas);
	void freeValue(char* claveDict,char* valueDict){
		free(valueDict);
	}
	dictionary_iterator(aux_compactacion, freeValue);
	dictionary_clean(aux_compactacion);
	if(condicion){
		EnviarDatosTipo(socketCoordinador,INSTANCIA,NULL,0,INICIOCOMPACTACION);
	}else{
		EnviarDatosTipo(socketCoordinador,INSTANCIA,NULL,0,COMPACTACIONOK);
	}
}

void aplicarAlgoritmoReemplazo(int cantidadEntradas) {
	if (!strcmp(ALGORITMO_REEMPLAZO, "CIRC")){
		int i = 0;
		int entradasAux=cantidadEntradas;
		t_list*atomicos=list_create();
		atomicos = list_filter(entradas_administrativa,LAMBDA(int _(t_Entrada *e) {return e->atomico && e->activo;}));
		if (list_size(atomicos) > 0 && cantidadEntradas<=list_size(atomicos)) {
			t_Entrada *aux = list_get(atomicos, i);
			if (entradasAux > 1) {
				while (entradasAux) {
					strcpy(tabla_entradas[aux->index],"NaN");
					t_Entrada *elem = list_remove_by_condition(entradas_administrativa, LAMBDA(int _(t_Entrada *e) {return e->index == aux->index;}));
					EnviarDatosTipo(socketCoordinador, INSTANCIA, elem->clave,strlen(elem->clave) + 1, ELIMINARCLAVE);
					entradasAux--;
					i++;
					aux = list_get(atomicos, i);
					free(elem->clave);
					free(elem);
				}
			} else {
				//borro entrada actual
				strcpy(tabla_entradas[aux->index], "NaN");
				//t_Entrada *elem=list_remove_by_condition(atomicos,LAMBDA(int _(t_Entrada *e) {return e->index == aux->index;}));
				t_Entrada* elem=list_remove_by_condition(entradas_administrativa,LAMBDA(int _(t_Entrada *e) {return e->index == aux->index;}));
				EnviarDatosTipo(socketCoordinador, INSTANCIA, elem->clave,strlen(elem->clave) + 1, ELIMINARCLAVE);
				free(elem->clave);
				free(elem);
			}
		} else {
			if (ENTRADAS_LIBRES >= entradasAux) {
				compactacion(true);
			} else {
				printf("Nose\n");
			}
		}
		list_destroy(atomicos);
	} else if (!strcmp(ALGORITMO_REEMPLAZO, "LRU")) {
//		El algoritmo LRU se basa en llevar registro de hace cuánto
//		fue referenciada cada entrada. Llegado el momento de reemplazar una entrada,
//		se selecciona aquella entrada que ha sido referenciada hace mayor tiempo(^17)
//		^17: Para simplificar el manejo de referencias sólo se llevará cuenta de hace cuantas
//		operaciones fue referenciada cada entrada, almacenando el número de ultima referencia y
//		reemplazando el menor. Esto solo sera efectivo si se accedió a la instancia (SET y STORE),
//		caso contrario no afectara el calculo del algoritmo.

//		se agrega atributo 'ultimaReferencia'(int) a t_entrada
//		hacer lista global de t_entrada y sumar 1 a ultimaReferencia por cada SET/STORE que realizo la instancia
//		reemplazar entrada que mayor valor de 'ultimaReferencia' tiene
	} else if (!strcmp(ALGORITMO_REEMPLAZO, "BSU")) {
//		lleva registro del tamaño dentro de la entrada atomica que está siendo ocupado, y en el momento de un reemplazo,
//		escoge aquél que ocupa más espacio dentro de una entrada.

//		Fíjate que esa lista global de instancias tiene un atributo que se llama tamanio
//		Y ahí tiene el tamaño del string que se guarda
//		Seria algo asi como ordenar por eso la lista y obtener el primero
	}
}

int ceilDivision(int lengthValue) {
	double cantidadEntradas;
	cantidadEntradas = (lengthValue + TAMANIO_ENTRADA - 1) / TAMANIO_ENTRADA;
	return cantidadEntradas;
}

int getFirstIndex(int entradasValue) {
	int i;
	for (i = 0; i < CANT_ENTRADA; i++) {
		if(entradasValue==1){
			if(!strcmp(tabla_entradas[i],"NaN")){
				return i;
			}
		}else{
			if (!strcmp(tabla_entradas[i], "NaN") && i+(entradasValue - 1) < CANT_ENTRADA) {
				int aux;
				bool cumple = true;
				//evaluo valores intermedios entre el inicio y el supuesto final (entradasValue-1)
				for (aux = i + 1; aux < i+entradasValue; aux++) {
					if (tabla_entradas[aux] && strcmp(tabla_entradas[aux], "NaN")) {
						cumple = false;
						break;
					}
				}
				if (cumple){
					return i;
				}
			}
		}
	}
	if (ENTRADAS_LIBRES >= entradasValue) {
		/*Cuando se lanza una compactación en una Instancia, se tiene
		 * que hacer en todas las demas. Falta enviar un mensaje al coordinador*/
		compactacion(true);
	} else {
		aplicarAlgoritmoReemplazo(entradasValue);
	}
	for (i = 0; i < CANT_ENTRADA; i++) {
		if(entradasValue==1){
			if(!strcmp(tabla_entradas[i],"NaN")){
				return i;
			}
		}else{
			if (!strcmp(tabla_entradas[i], "NaN") && i+(entradasValue - 1) < CANT_ENTRADA) {
				int aux;
				bool cumple = true;
				//evaluo valores intermedios entre el inicio y el supuesto final (entradasValue-1)
				for (aux = i + 1; aux < i+entradasValue; aux++) {
					if (tabla_entradas[aux] && strcmp(tabla_entradas[aux], "NaN")) {
						cumple = false;
						break;
					}
				}
				if (cumple){
					return i;
				}
			}
		}
	}
	return -1;
}
void obtenerEntradasViejas(){
	DIR* dir = opendir(PUNTO_MONTAJE);
	struct dirent *ent;
	while ((ent = readdir (dir)) != NULL) {
		if(strcmp(ent->d_name,".") && strcmp(ent->d_name,"..")){
			char* directorio=malloc(strlen(PUNTO_MONTAJE)+1+strlen(ent->d_name)+1);
			strcpy(directorio,PUNTO_MONTAJE);
			strcat(directorio,"/");
			strcat(directorio,ent->d_name);
			char* key =malloc(strlen(ent->d_name)+1);
			strcpy(key,ent->d_name);
			FILE *f= fopen(directorio,"r");
			fseek(f, 0, SEEK_END);
			int fsize = ftell(f);
			fseek(f, 0, SEEK_SET);
			char *value = malloc(fsize+1);
			fread(value,fsize,1,f);
			fclose(f);
			t_Entrada *nueva = malloc(sizeof(t_Entrada));
			nueva->clave = malloc(strlen(key) + 1);
			strcpy(nueva->clave, key);
			nueva->entradasOcupadas = ceilDivision(strlen(value));
			nueva->tamanio = strlen(value);
			nueva->index = getFirstIndex(nueva->entradasOcupadas);
			nueva->atomico =TAMANIO_ENTRADA - nueva->tamanio >= 0 ? true : false;
			nueva->activo=true;
			list_add(entradas_administrativa, nueva);
			int i,j=nueva->tamanio;
			for (i = nueva->index; i < (nueva->index + nueva->entradasOcupadas);i++) {
				if ((nueva->index + nueva->entradasOcupadas) - 1 == i) {
					strcpy(tabla_entradas[i], value);
					value+=j;
					value-=nueva->tamanio;
					ENTRADAS_LIBRES--;
					break;
				}
				strncpy(tabla_entradas[i], value, TAMANIO_ENTRADA);
				value += TAMANIO_ENTRADA;
				j-=TAMANIO_ENTRADA;
				ENTRADAS_LIBRES--;
			}
			free(directorio);
			free(key);
			free(value);
			//free(valueAux);
		}
	}
	closedir(dir);
}

void verificarPuntoMontaje() {
	DIR* dir = opendir(PUNTO_MONTAJE);
	if (dir) {
		  closedir (dir);
	} else if (ENOENT == errno) {
		//el directorio no existe
		mkdir(PUNTO_MONTAJE, 0700);
	} else {
		log_error(logger,"Se detectó el siguiente error al abrir el directorio: %s",strerror(errno));
		printf("Fallo el opendir \n");
		fflush(stdout);
	}
}

void crearArchivo(char*key, char*value) {
	char *ruta = malloc(strlen(PUNTO_MONTAJE) + strlen("/") + strlen(key) + 1);
	strcpy(ruta, PUNTO_MONTAJE);
	strcat(ruta, "/");
	strcat(ruta, key);
	FILE* f = fopen(ruta, "w");
	fwrite(value, 1, strlen(value) + 1, f);
	fclose(f);
	free(ruta);
}

//No se puso en el SWITCH debido a que es PROPIO DE LA INSTANCIA! No depende del COORDINADOR.

void dump() {
	pthread_mutex_lock(&mutex_entradas);

		t_list*atomicos=list_create();
		atomicos = list_filter(entradas_administrativa,LAMBDA(int _(t_Entrada *e) {return e->atomico;}));
		int i, j;
		for (i = 0; i < list_size(atomicos); i++) {

			t_Entrada* actual = (t_Entrada*) list_get(atomicos,i);
			char* directorio_actual = malloc(strlen(PUNTO_MONTAJE) + strlen(actual->clave) + 3);
			strcpy(directorio_actual, PUNTO_MONTAJE);
			strcat(directorio_actual,"/");
			strcat(directorio_actual, actual->clave);
			char* valor = malloc(actual->tamanio+1);
			int tamanioPegado = 0;
			for (j = actual->index;j < (actual->index + actual->entradasOcupadas); j++) {
				if ((actual->index + actual->entradasOcupadas) - 1 == j) {
					strcpy(valor + tamanioPegado, tabla_entradas[j]);
				} else {
					strcpy(valor + tamanioPegado, tabla_entradas[j]);
					tamanioPegado += TAMANIO_ENTRADA;
				}
			}
			FILE* file_a_crear = fopen(directorio_actual, "w");
			fwrite(valor, 1, strlen(valor)+1, file_a_crear);
			free(valor);
			fclose(file_a_crear);
			free(directorio_actual);
		}
		pthread_mutex_unlock(&mutex_entradas);
}

void ejecutarDump(){
	while(1){
		sleep(INTERVALO_DUMP);
		//dump();
	}
}

int main(int argc, char* argv[]) {
	obtenerValoresArchivoConfiguracion(argv[1]);
	imprimirArchivoConfiguracion();
	crearLogger();
	verificarPuntoMontaje();
	entradas_administrativa = list_create();
	aux_compactacion = dictionary_create();
	pthread_mutex_init(&mutex_entradas,NULL);
	socketCoordinador = ConectarAServidor(PUERTO_COORDINADOR, IP_COORDINADOR,COORDINADOR, INSTANCIA, RecibirHandshake);
	pthread_t hiloDump;
	pthread_create(&hiloDump, NULL, (void*) ejecutarDump, NULL);
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteCliente(socketCoordinador, INSTANCIA, &paquete) > 0) {
		datos = paquete.Payload;
		switch (paquete.header.tipoMensaje) {
		case SOLICITUDNOMBRE: {
			EnviarDatosTipo(socketCoordinador, INSTANCIA, NOMBRE_INSTANCIA,
					strlen(NOMBRE_INSTANCIA) + 1, IDENTIFICACIONINSTANCIA);
		}
			break;
		case STOREINST: {

			char *key = malloc(strlen(datos) + 1);
			strcpy(key, datos);
			if(!strcmp(key,"cocina:tallarines")){
				int a=3;
			}
			pthread_mutex_lock(&mutex_entradas);
			t_Entrada *esperada = list_find(entradas_administrativa,LAMBDA(int _(t_Entrada *elemento) {
						return !strcmp(key, elemento->clave);
					}
			));
			//ya no esta mas activa esta entrada
			esperada->activo=false;
			//obtengo el valor asociado
			char *valueReturn = malloc(esperada->tamanio + 1);
			for (int var = esperada->index;var < esperada->index + esperada->entradasOcupadas; var++) {
				if ((esperada->index + esperada->entradasOcupadas) - 1 == var) {
					strcpy(valueReturn, tabla_entradas[var]);
					valueReturn += strlen(tabla_entradas[var]);
					break;
				}
				strncpy(valueReturn, tabla_entradas[var], TAMANIO_ENTRADA);
				valueReturn += TAMANIO_ENTRADA;
			}
			valueReturn -= esperada->tamanio;
			if(esperada->tamanio==20){
				valueReturn--;
				char*aux=malloc(20);
				strncpy(aux,valueReturn,20);
				valueReturn=aux;
			}
			pthread_mutex_unlock(&mutex_entradas);
			int i;
			//creo el archivo
			crearArchivo(key, valueReturn);
			//limpio los datos
			for (i = esperada->index; i < (esperada->index + esperada->entradasOcupadas);i++) {
				strcpy(tabla_entradas[i],"NaN");
				ENTRADAS_LIBRES++;
			}
			log_info(logger, "STORE OK se creo archivo con clave %s y valor %s",key, valueReturn);
			//aviso a coordinador que termine OK
			EnviarDatosTipo(socketCoordinador, INSTANCIA, key, strlen(key) + 1,STOREOK);
			free(key);
			free(valueReturn);
		}
			break;
		case SETINST: {
			char*key = malloc(strlen(datos) + 1);
			strcpy(key, datos);
			datos += strlen(datos) + 1;
			char* value = malloc(strlen(datos) + 1);
			strcpy(value, datos);
			pthread_mutex_lock(&mutex_entradas);
			if (NULL == list_find(entradas_administrativa,LAMBDA(bool _(t_Entrada *elemento) { return !strcmp(key, elemento->clave);})))
			{
				t_Entrada *nueva = malloc(sizeof(t_Entrada));
				nueva->clave = malloc(strlen(key) + 1);
				strcpy(nueva->clave, key);
				nueva->entradasOcupadas = ceilDivision(strlen(value));
				nueva->tamanio = strlen(value);
				nueva->index = getFirstIndex(nueva->entradasOcupadas);
				nueva->atomico =TAMANIO_ENTRADA - nueva->tamanio >= 0 ? true : false;
				nueva->activo=true;
				list_add(entradas_administrativa, nueva);
				int i;
				char *valueAux = malloc(strlen(value) + 1);
				strcpy(valueAux, value);
				for (i = nueva->index; i < (nueva->index + nueva->entradasOcupadas);i++) {
					if ((nueva->index + nueva->entradasOcupadas) - 1 == i) {
						//Porque no puedo hacer un free de tabla_entradas[i]?
						strcpy(tabla_entradas[i], valueAux);
						ENTRADAS_LIBRES--;
						break;
					}
					strncpy(tabla_entradas[i], valueAux, TAMANIO_ENTRADA);
					valueAux += TAMANIO_ENTRADA;
					ENTRADAS_LIBRES--;
				}
			}
			else
			{
				t_Entrada* entrada = list_find(entradas_administrativa,LAMBDA(bool _(t_Entrada *elemento) { return !strcmp(key, elemento->clave);}));
				int i;
				for (i = entrada->index; i < (entrada->index + entrada->entradasOcupadas);
						i++) {
					strcpy(tabla_entradas[i],"NaN");
					ENTRADAS_LIBRES++;
				}

				entrada->entradasOcupadas = ceilDivision(strlen(value));
				entrada->tamanio = strlen(value);
				entrada->index = getFirstIndex(entrada->entradasOcupadas);
				entrada->atomico =TAMANIO_ENTRADA - entrada->tamanio >= 0 ? true : false;
				entrada->activo = true;
				list_add(entradas_administrativa, entrada);
				char *valueAux = malloc(strlen(value) + 1);
				strcpy(valueAux, value);
				for (i = entrada->index; i < (entrada->index + entrada->entradasOcupadas);
						i++) {
					if ((entrada->index + entrada->entradasOcupadas) - 1 == i) {
						//Porque no puedo hacer un free de tabla_entradas[i]?
						tabla_entradas[i] = malloc(TAMANIO_ENTRADA);
						strcpy(tabla_entradas[i], valueAux);
						ENTRADAS_LIBRES--;
						break;
					}
					strncpy(tabla_entradas[i], valueAux, TAMANIO_ENTRADA);
					valueAux += TAMANIO_ENTRADA;
					ENTRADAS_LIBRES--;
				}
			}
			pthread_mutex_unlock(&mutex_entradas);
			log_info(logger, "se hizo un SET de clave %s", key);
			void* claveykey = malloc(strlen(key)+strlen(value)+2);
			strcpy(claveykey,key);
			strcpy(claveykey+strlen(key)+1,value);
			EnviarDatosTipo(socketCoordinador, INSTANCIA, claveykey, strlen(key) + strlen(value) + 2,
					SETOK);
			free(claveykey);
			free(key);

			free(value);

		}
			break;
		case GETENTRADAS: {
			TAMANIO_ENTRADA = *((int*) datos);
			datos += sizeof(int);
			CANT_ENTRADA = *((int*) datos);
			datos += sizeof(int);
			tabla_entradas = malloc(CANT_ENTRADA * sizeof(char*)); //CANT_ENTRADA * TAMANIO_ENTRADA
			int i;
			for (i = 0; i < CANT_ENTRADA; i++) {
				tabla_entradas[i] = malloc(TAMANIO_ENTRADA);
				strcpy(tabla_entradas[i], "NaN");
			}
			ENTRADAS_LIBRES=CANT_ENTRADA;
			obtenerEntradasViejas();
		}
			break;
		case COMPACTACION: {
			compactacion(false);
		}
			break;
		}
		if (paquete.Payload != NULL) {
			free(paquete.Payload);
		}
	}
	return EXIT_SUCCESS;
}

