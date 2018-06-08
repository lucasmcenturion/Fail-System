#include "sockets.h"
#include <dirent.h>
#include <commons/collections/list.h>

char *IP_COORDINADOR, *ALGORITMO_REEMPLAZO, *PUNTO_MONTAJE, *NOMBRE_INSTANCIA;
int PUERTO_COORDINADOR, INTERVALO_DUMP, TAMANIO_ENTRADA, CANT_ENTRADA,
		ENTRADAS_LIBRES, socketCoordinador;
char **tabla_entradas;
t_list *entradas_administrativa;
t_log * logger;
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
	/*PUNTO_MONTAJE = string_duplicate(
			config_get_string_value(arch, "PUNTO_MONTAJE"));
	NOMBRE_INSTANCIA = string_duplicate(
			config_get_string_value(arch, "NOMBRE_INSTANCIA"));*/
	PUNTO_MONTAJE = string_from_format("/home/utnso/Instancia%s", id);
	NOMBRE_INSTANCIA = string_from_format("Instancia%s", id);
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
void compactacion() {
	//todo
}
void aplicarAlgoritmoReemplazo(int cantidadEntradas) {
	int i = 0;
	t_list*atomicos=list_create();
	atomicos = list_filter(entradas_administrativa,LAMBDA(int _(t_Entrada *e) {return e->atomico;}));
	if (list_size(atomicos) > 0) {
		t_Entrada *aux = list_get(atomicos, i);
		if (cantidadEntradas > 1) {
			while (cantidadEntradas) {
				strcpy(tabla_entradas[aux->index],"NaN");
				t_Entrada *elem = list_remove_by_condition(entradas_administrativa, LAMBDA(int _(t_Entrada *e) {return e->index == aux->index;}));
				free(elem->clave);
				free(elem);
				if (cantidadEntradas - 1 > 0) {
					i++;
					aux = list_get(atomicos, i);
					cantidadEntradas--;
				}
			}
		} else {
			//borro entrada actual
			strcpy(tabla_entradas[aux->index], "NaN");
			t_Entrada *elem=list_remove_by_condition(atomicos,LAMBDA(int _(t_Entrada *e) {return e->index == aux->index;}));
			EnviarDatosTipo(socketCoordinador, INSTANCIA, elem->clave,strlen(elem->clave) + 1, ELIMINARCLAVE);
			free(elem->clave);
			free(elem);
		}
	} else {
		if (ENTRADAS_LIBRES >= cantidadEntradas) {
			compactacion();
		} else {
			printf("Nose\n");
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
		if (!strcmp(tabla_entradas[i], "NaN")
				&& tabla_entradas[entradasValue - 1]) {
			int aux;
			bool cumple = true;
			//evaluo valores intermedios entre el inicio y el supuesto final (entradasValue-1)
			for (aux = i + 1; aux < entradasValue; aux++) {
				if (strcmp(tabla_entradas[aux], "NaN")) {
					cumple = false;
					break;
				}
			}
			if (cumple)
				return i;
		}
	}
	//no tiene espacio, aplicar algoritmo
	aplicarAlgoritmoReemplazo(entradasValue);
	for (i = 0; i < CANT_ENTRADA; i++) {
		if (!strcmp(tabla_entradas[i], "NaN")
				&& tabla_entradas[entradasValue - 1]) {
			int aux;
			bool cumple = true;
			//evaluo valores intermedios entre el inicio y el supuesto final (entradasValue-1)
			for (aux = i + 1; aux < entradasValue; aux++) {
				if (strcmp(tabla_entradas[aux], "NaN")) {
					cumple = false;
					break;
				}
			}
			if (cumple)
				return i;
		}
	}
	return -1;
}
void verificarPuntoMontaje() {

	DIR* dir = opendir(PUNTO_MONTAJE);
	if (dir) {
		/* Directory exists. */
		/* El directorio existe. Hay que recorrer todos sus archivos y por cada uno de ellos
		 *  verificar el nombre del archivo(clave) y lo que haya dentro es el value(valor)
		 * Despues hay que añadirlo a la tabla de entradas y mandarle un mensaje a COORDINADOR con las entradas que tiene */
		closedir(dir);
	} else if (ENOENT == errno) {
		//el directorio no existe
//		log_info(vg_logger,"No existe el punto de montaje, se crea el directorio");
		mkdir(PUNTO_MONTAJE, 0700);
	} else {
		log_error(logger,
				"Se detectó el siguiente error al abrir el directorio: %s",
				strerror(errno));
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
		dump();
	}
}

int main(int argc, char* argv[]) {
	obtenerValoresArchivoConfiguracion(argv[1]);
	imprimirArchivoConfiguracion();
	crearLogger();
	ENTRADAS_LIBRES = CANT_ENTRADA;
	verificarPuntoMontaje();
	entradas_administrativa = list_create();
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
			pthread_mutex_lock(&mutex_entradas);
			t_Entrada *esperada = list_find(entradas_administrativa,
					LAMBDA(int _(t_Entrada *elemento) {
						return !strcmp(key, elemento->clave);
					}
			));
			esperada->activo=false;
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
			pthread_mutex_unlock(&mutex_entradas);
			crearArchivo(key, valueReturn);
			log_info(logger, "STORE OK se creo archivo con clave %s y valor %s",key, valueReturn);
			EnviarDatosTipo(socketCoordinador, INSTANCIA, key, strlen(key) + 1,
					STOREOK);
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
				list_add(entradas_administrativa, nueva);  //
				int i;
				char *valueAux = malloc(strlen(value) + 1);
				strcpy(valueAux, value);
				for (i = nueva->index; i < (nueva->index + nueva->entradasOcupadas);
						i++) {
					if ((nueva->index + nueva->entradasOcupadas) - 1 == i) {
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
			else
			{
				t_Entrada* entrada = list_find(entradas_administrativa,LAMBDA(bool _(t_Entrada *elemento) { return !strcmp(key, elemento->clave);}));
				int i;
				for (i = entrada->index; i < (entrada->index + entrada->entradasOcupadas);
						i++) {
					strcpy(tabla_entradas[i],"NaN");
				}

				entrada->entradasOcupadas = ceilDivision(strlen(value));
				entrada->tamanio = strlen(value);
				entrada->index = getFirstIndex(entrada->entradasOcupadas);
				entrada->atomico =TAMANIO_ENTRADA - entrada->tamanio >= 0 ? true : false;
				entrada->activo = true;
				list_add(entradas_administrativa, entrada);  //

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
			EnviarDatosTipo(socketCoordinador, INSTANCIA, key, strlen(key) + 1,
					SETOK);
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
		}
			break;
		}
		if (paquete.Payload != NULL) {
			free(paquete.Payload);
		}
	}
	return EXIT_SUCCESS;
}

