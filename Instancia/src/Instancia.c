#include "sockets.h"
#include <dirent.h>
#include <commons/collections/list.h>

char *IP_COORDINADOR, *ALGORITMO_REEMPLAZO, *PUNTO_MONTAJE, *NOMBRE_INSTANCIA;
int PUERTO_COORDINADOR, INTERVALO_DUMP, TAMANIO_ENTRADA, CANT_ENTRADA, ULT_REF,
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
	INTERVALO_DUMP = (strcmp(id,"1") ? 100 : 10); //config_get_int_value(arch, "INTERVALO_DUMP");
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
	log_info(logger,"Se activo compactación en %s,comenzando la compactación...",NOMBRE_INSTANCIA);
	void insertarEnDictionary(t_Entrada * aux){
		//obtenemos el valor asociado
		char *valueReturn = calloc(1,aux->tamanio + 1);
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
			free(tabla_entradas[i]);
			tabla_entradas[i]=calloc(1,TAMANIO_ENTRADA);
			strcpy(tabla_entradas[i],"NaN");
		}

	}

	t_list*activos= list_filter(entradas_administrativa,LAMBDA(int _(t_Entrada *ent) {return ent->activo;}));
	list_iterate(activos, insertarEnDictionary);

	void reOrdenarEntradas(t_Entrada *e){
		//nuevo index
		e->index = getFirstIndex(e->entradasOcupadas);
		char *valueAux = calloc(1,strlen(dictionary_get(aux_compactacion,e->clave)) + 1);
		strcpy(valueAux, dictionary_get(aux_compactacion,e->clave));
		int j;
		//copio en tabla entradas posta
		for (j = e->index; j < (e->index + e->entradasOcupadas);j++) {
			if ((e->index + e->entradasOcupadas) - 1 == j) {
				strcpy(tabla_entradas[j], valueAux);
				valueAux+=strlen(tabla_entradas[j]);
				break;
			}
			strncpy(tabla_entradas[j], valueAux, TAMANIO_ENTRADA);
			valueAux += TAMANIO_ENTRADA;
		}
		valueAux-=e->tamanio;
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
	log_info(logger,"Compactación en %s finalizada",NOMBRE_INSTANCIA);
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
					int tam = strlen(elem->clave)+1+sizeof(int);
					void* claveYTamanio = calloc(1,tam);
					strcpy(claveYTamanio,elem->clave);
					claveYTamanio+=strlen(elem->clave)+1;
					*((int*)claveYTamanio)=elem->entradasOcupadas;
					claveYTamanio+=sizeof(int);
					claveYTamanio-=tam;
					EnviarDatosTipo(socketCoordinador, INSTANCIA, claveYTamanio,tam, ELIMINARCLAVE);
					free(claveYTamanio);
					entradasAux--;
					i++;
					aux = list_get(atomicos, i);
					log_info(logger,"Se reemplazo la clave %s",elem->clave);
					//free(elem->clave);
					//free(elem);
				}
			} else {
				//borro entrada actual
				strcpy(tabla_entradas[aux->index], "NaN");
				//t_Entrada *elem=list_remove_by_condition(atomicos,LAMBDA(int _(t_Entrada *e) {return e->index == aux->index;}));
				t_Entrada* elem=list_remove_by_condition(entradas_administrativa,LAMBDA(int _(t_Entrada *e) {return e->index == aux->index;}));
				int tam = strlen(elem->clave)+1+sizeof(int);
				void* claveYTamanio = calloc(1,tam);
				strcpy(claveYTamanio,elem->clave);
				claveYTamanio+=strlen(elem->clave)+1;
				*((int*)claveYTamanio)=elem->entradasOcupadas;
				claveYTamanio+=sizeof(int);
				claveYTamanio-=tam;
				EnviarDatosTipo(socketCoordinador, INSTANCIA, claveYTamanio,tam, ELIMINARCLAVE);
				free(claveYTamanio);
				log_info(logger,"Se reemplazo la clave %s",elem->clave);
				//free(elem->clave);
				// free(elem);
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

		//funcion para comparar ultimasReferencias
		bool ComparadorDeRef(t_Entrada* entrada, t_Entrada* entradaMenosRef) {
			return entrada->utlimaReferencia < entradaMenosRef->utlimaReferencia;
		}
		//creo lista atomicos
		t_list*atomicos=list_create();
		atomicos = list_filter(entradas_administrativa,LAMBDA(int _(t_Entrada *e) {return e->atomico && e->activo;}));

		//like algoritmo de reemplazo CIRC nocierto? copypaste pero con atomicos ordenado por ultReferencia
		int entradasAux=cantidadEntradas;
		int i = 0;
		if (list_size(atomicos) > 0 && cantidadEntradas<=list_size(atomicos)) {
			//ordeno lista atomicos por ultimaReferencia de menor a mayor (atomicos[0] -> menor ref)
			list_sort(atomicos, (void*) ComparadorDeRef);

			t_Entrada *aux = list_get(atomicos, i);
			if (entradasAux > 1) {
				while (entradasAux) {
					strcpy(tabla_entradas[aux->index],"NaN");
					t_Entrada *elem = list_remove_by_condition(entradas_administrativa, LAMBDA(int _(t_Entrada *e) {return e->index == aux->index;}));
					int tam = strlen(elem->clave)+1+sizeof(int);
					void* claveYTamanio = calloc(1,tam);
					strcpy(claveYTamanio,elem->clave);
					claveYTamanio+=strlen(elem->clave)+1;
					*((int*)claveYTamanio)=elem->entradasOcupadas;
					claveYTamanio+=sizeof(int);
					claveYTamanio-=tam;
					EnviarDatosTipo(socketCoordinador, INSTANCIA, claveYTamanio,tam, ELIMINARCLAVE);
					free(claveYTamanio);
					entradasAux--;
					i++;
					aux = list_get(atomicos, i);
					log_info(logger,"Se reemplazo la clave %s",elem->clave);
//					free(elem->clave);
//					free(elem);
				}
			} else {
				//borro entrada actual
				strcpy(tabla_entradas[aux->index], "NaN");
				//t_Entrada *elem=list_remove_by_condition(atomicos,LAMBDA(int _(t_Entrada *e) {return e->index == aux->index;}));
				t_Entrada* elem=list_remove_by_condition(entradas_administrativa,LAMBDA(int _(t_Entrada *e) {return e->index == aux->index;}));
				int tam = strlen(elem->clave)+1+sizeof(int);
				void* claveYTamanio = calloc(1,tam);
				strcpy(claveYTamanio,elem->clave);
				claveYTamanio+=strlen(elem->clave)+1;
				*((int*)claveYTamanio)=elem->entradasOcupadas;
				claveYTamanio+=sizeof(int);
				claveYTamanio-=tam;
				EnviarDatosTipo(socketCoordinador, INSTANCIA, claveYTamanio,tam, ELIMINARCLAVE);
				free(claveYTamanio);
				log_info(logger,"Se reemplazo la clave %s",elem->clave);
//				free(elem->clave);
//				free(elem);
			}
		} else {
			if (ENTRADAS_LIBRES >= entradasAux) {
				compactacion(true);
			} else {
				printf("Nose\n");
			}
		}
		//fin copypaste
	} else if (!strcmp(ALGORITMO_REEMPLAZO, "BSU")) {
		bool ComparadorDeTamanio(t_Entrada* entrada, t_Entrada* entradaMenosTam) {
			return entrada->tamanio > entradaMenosTam->tamanio;
		}
		//creo lista atomicos
		t_list*atomicos=list_create();
		atomicos = list_filter(entradas_administrativa,LAMBDA(int _(t_Entrada *e) {return e->atomico && e->activo;}));

		//like algoritmo de reemplazo CIRC nocierto? copypaste pero con atomicos ordenado por tamanio
		int entradasAux=cantidadEntradas;
		int i = 0;
		if (list_size(atomicos) > 0 && cantidadEntradas<=list_size(atomicos)) {
			//ordeno lista atomicos por tamanio de entrada de mayor a menor (atomicos[0] -> mayor tamanio)
			list_sort(atomicos, (void*) ComparadorDeTamanio);

			t_Entrada *aux = list_get(atomicos, i);
			if (entradasAux > 1) {
				while (entradasAux) {
					strcpy(tabla_entradas[aux->index],"NaN");
					t_Entrada *elem = list_remove_by_condition(entradas_administrativa, LAMBDA(int _(t_Entrada *e) {return e->index == aux->index;}));
					int tam = strlen(elem->clave)+1+sizeof(int);
					void* claveYTamanio = calloc(1,tam);
					strcpy(claveYTamanio,elem->clave);
					claveYTamanio+=strlen(elem->clave)+1;
					*((int*)claveYTamanio)=elem->entradasOcupadas;
					claveYTamanio+=sizeof(int);
					claveYTamanio-=tam;
					EnviarDatosTipo(socketCoordinador, INSTANCIA, claveYTamanio,tam, ELIMINARCLAVE);
					free(claveYTamanio);
					entradasAux--;
					i++;
					aux = list_get(atomicos, i);
					log_info(logger,"Se reemplazo la clave %s",elem->clave);
//					free(elem->clave);
//					free(elem);
				}
			} else {
				//borro entrada actual
				strcpy(tabla_entradas[aux->index], "NaN");
				//t_Entrada *elem=list_remove_by_condition(atomicos,LAMBDA(int _(t_Entrada *e) {return e->index == aux->index;}));
				t_Entrada* elem=list_remove_by_condition(entradas_administrativa,LAMBDA(int _(t_Entrada *e) {return e->index == aux->index;}));
				int tam = strlen(elem->clave)+1+sizeof(int);
				void* claveYTamanio = calloc(1,tam);
				strcpy(claveYTamanio,elem->clave);
				claveYTamanio+=strlen(elem->clave)+1;
				*((int*)claveYTamanio)=elem->entradasOcupadas;
				claveYTamanio+=sizeof(int);
				claveYTamanio-=tam;
				EnviarDatosTipo(socketCoordinador, INSTANCIA, claveYTamanio,tam, ELIMINARCLAVE);
				free(claveYTamanio);
				log_info(logger,"Se reemplazo la clave %s",elem->clave);
//				free(elem->clave);
//				free(elem);
			}
		} else {
			if (ENTRADAS_LIBRES >= entradasAux) {
				compactacion(true);
			} else {
				printf("Nose\n");
			}
		}
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
			char* directorio=calloc(1,strlen(PUNTO_MONTAJE)+1+strlen(ent->d_name)+1);
			strcpy(directorio,PUNTO_MONTAJE);
			strcat(directorio,"/");
			strcat(directorio,ent->d_name);
			char* key =calloc(1,strlen(ent->d_name)+1);
			strcpy(key,ent->d_name);
			FILE *f= fopen(directorio,"r");
			fseek(f, 0, SEEK_END);
			int fsize = ftell(f);
			fseek(f, 0, SEEK_SET);
			char *value = calloc(1,fsize+1);
			fread(value,fsize,1,f);
			fclose(f);
			t_Entrada *nueva = calloc(1,sizeof(t_Entrada));
			nueva->clave = calloc(1,strlen(key) + 1);
			strcpy(nueva->clave, key);
			nueva->entradasOcupadas = ceilDivision(strlen(value));
			nueva->tamanio = strlen(value);
			nueva->index = getFirstIndex(nueva->entradasOcupadas);
			nueva->atomico =TAMANIO_ENTRADA - nueva->tamanio >= 0 ? true : false;
			nueva->activo=true;
			nueva->utlimaReferencia = 0;
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
	char *ruta = calloc(1,strlen(PUNTO_MONTAJE) + strlen("/") + strlen(key) + 1);
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
			char* directorio_actual = calloc(1,strlen(PUNTO_MONTAJE) + strlen(actual->clave) + 3);
			strcpy(directorio_actual, PUNTO_MONTAJE);
			strcat(directorio_actual,"/");
			strcat(directorio_actual, actual->clave);
			char* valor = calloc(1,actual->tamanio+1);
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
		dump();
	}
}

int main(int argc, char* argv[]) {
	obtenerValoresArchivoConfiguracion(argv[1]);
	imprimirArchivoConfiguracion();
	crearLogger();
	verificarPuntoMontaje();
	entradas_administrativa = list_create();
	aux_compactacion = dictionary_create();
	ULT_REF=0;
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
			printf("%s\n",tabla_entradas[0]);
			fflush(stdout);
			char *key = calloc(1,strlen(datos) + 1);
			strcpy(key, datos);
			pthread_mutex_lock(&mutex_entradas);
			t_Entrada *esperada = list_find(entradas_administrativa,LAMBDA(int _(t_Entrada *elemento) {
						return !strcmp(key, elemento->clave);
					}
			));
			//ya no esta mas activa esta entrada
			esperada->activo=false;
			//obtengo el valor asociado
			printf("ANTES DE GUARDAR %s\n",tabla_entradas[0]);
			fflush(stdout);
			char *valueReturn = calloc(1,esperada->tamanio + 1);
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
			printf("DESPUES DE GUARDAR %s\n",tabla_entradas[0]);
			fflush(stdout);
			pthread_mutex_unlock(&mutex_entradas);
			int i;
			//creo el archivo
			crearArchivo(key, valueReturn);
			//limpio los datos
			printf("ANTES DE LIMPIAR %s\n",tabla_entradas[0]);
			fflush(stdout);
			for (i = esperada->index; i < (esperada->index + esperada->entradasOcupadas);i++) {
				strcpy(tabla_entradas[i],"NaN");
				ENTRADAS_LIBRES++;
			}
			printf("DESPUES DE LIMPIAR %s\n",tabla_entradas[0]);
			fflush(stdout);
			log_info(logger, "STORE OK se creo archivo con clave %s y valor %s",key, valueReturn);
			//aviso a coordinador que termine OK
			EnviarDatosTipo(socketCoordinador, INSTANCIA, key, strlen(key) + 1,STOREOK);
			free(key);
			free(valueReturn);
		}
			break;
		case SETINST: {
			printf("%s\n",tabla_entradas[0]);
			fflush(stdout);
			char*key = calloc(1,100);
			strcpy(key, datos);
			key = realloc(key,strlen(key)+1);
			datos += strlen(datos) + 1;
			char* value = calloc(1,strlen(datos) + 1);
			strcpy(value, datos);
			pthread_mutex_lock(&mutex_entradas);
			bool iguales(t_Entrada*e){
				return !strcmp(key,e->clave) ? true : false;
			}
			int entradasOcupadas;
			if (NULL == list_find(entradas_administrativa,iguales))
			{
				t_Entrada *nueva = calloc(1,sizeof(t_Entrada));
				nueva->clave = calloc(1,strlen(key) + 1);
				strcpy(nueva->clave, key);
				nueva->entradasOcupadas = ceilDivision(strlen(value));
				entradasOcupadas=nueva->entradasOcupadas;
				nueva->tamanio = strlen(value);
				nueva->index = getFirstIndex(nueva->entradasOcupadas);
				nueva->atomico =TAMANIO_ENTRADA - nueva->tamanio >= 0 ? true : false;
				nueva->activo=true;
				nueva->utlimaReferencia=ULT_REF;
				list_add(entradas_administrativa, nueva);
				int i;
				char *valueAux = calloc(1,strlen(value) + 1);
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
				if(entrada->activo){
					for (i = entrada->index; i < (entrada->index + entrada->entradasOcupadas);
							i++) {
						free(tabla_entradas[i]);
						tabla_entradas[i]=calloc(1,TAMANIO_ENTRADA);
						strcpy(tabla_entradas[i],"NaN");
						ENTRADAS_LIBRES++;
					}
				}

				entrada->entradasOcupadas = ceilDivision(strlen(value));
				entradasOcupadas=entrada->entradasOcupadas;
				entrada->tamanio = strlen(value);
				entrada->index = getFirstIndex(entrada->entradasOcupadas);
				entrada->atomico =TAMANIO_ENTRADA - entrada->tamanio >= 0 ? true : false;
				entrada->activo = true;
				entrada->utlimaReferencia=ULT_REF;
				char *valueAux = calloc(1,strlen(value) + 1);
				strcpy(valueAux, value);
				for (i = entrada->index; i < (entrada->index + entrada->entradasOcupadas);
						i++) {
					if ((entrada->index + entrada->entradasOcupadas) - 1 == i) {
						tabla_entradas[i] = calloc(1,TAMANIO_ENTRADA);
						strcpy(tabla_entradas[i], valueAux);
						ENTRADAS_LIBRES--;
						break;
					}
					strncpy(tabla_entradas[i], valueAux, TAMANIO_ENTRADA);
					valueAux += TAMANIO_ENTRADA;
					ENTRADAS_LIBRES--;
				}
			}
			ULT_REF++;
			pthread_mutex_unlock(&mutex_entradas);
			log_info(logger, "se hizo un SET de clave %s", key);
			int tam=strlen(key)+strlen(value)+sizeof(int)+2;
			void* claveykey = calloc(1,tam);
			strcpy(claveykey,key);
			claveykey+=strlen(key)+1;
			strcpy(claveykey,value);
			claveykey+=strlen(value)+1;
			*((int*)claveykey)=entradasOcupadas;
			claveykey+=sizeof(int);
			claveykey-=tam;
			EnviarDatosTipo(socketCoordinador, INSTANCIA, claveykey, tam, SETOK);
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
			tabla_entradas = calloc(1,CANT_ENTRADA * sizeof(char*)); //CANT_ENTRADA * TAMANIO_ENTRADA
			int i;
			for (i = 0; i < CANT_ENTRADA; i++) {
				tabla_entradas[i] = calloc(1,TAMANIO_ENTRADA);
				strcpy(tabla_entradas[i], "NaN");
			}
			ENTRADAS_LIBRES=CANT_ENTRADA;
			//obtenerEntradasViejas();
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

