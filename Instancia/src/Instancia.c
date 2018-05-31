#include "sockets.h"
#include <dirent.h>
#include <commons/collections/list.h>

char *IP_COORDINADOR, *ALGORITMO_REEMPLAZO, *PUNTO_MONTAJE, *NOMBRE_INSTANCIA;
int PUERTO_COORDINADOR, INTERVALO_DUMP, TAMANIO_ENTRADA, CANT_ENTRADA;
char **tabla_entradas;
t_list *entradas_administrativa;
t_log * vg_logger;

void obtenerValoresArchivoConfiguracion() {
	t_config* arch = config_create("/home/utnso/workspace/tp-2018-1c-Fail-system/Instancia/instancia.cfg");
	IP_COORDINADOR = string_duplicate(config_get_string_value(arch, "IP_COORDINADOR"));
	PUERTO_COORDINADOR = config_get_int_value(arch, "PUERTO_COORDINADOR");
	ALGORITMO_REEMPLAZO = string_duplicate(config_get_string_value(arch, "ALGORITMO_REEMPLAZO"));
	PUNTO_MONTAJE = string_duplicate(config_get_string_value(arch, "PUNTO_MONTAJE"));
	NOMBRE_INSTANCIA = string_duplicate(config_get_string_value(arch, "NOMBRE_INSTANCIA"));
	INTERVALO_DUMP = config_get_int_value(arch, "INTERVALO_DUMP");
	config_destroy(arch);
}

void imprimirArchivoConfiguracion(){
	printf(
				"Configuración:\n"
				"IP_COORDINADOR=%s\n"
				"PUERTO_COORDINADOR=%d\n"
				"ALGORITMO_REEMPLAZO=%s\n"
				"PUNTO_MONTAJE=%s\n"
				"NOMBRE_INSTANCIA=%s\n"
				"INTERVALO_DUMP=%d\n",
				IP_COORDINADOR,
				PUERTO_COORDINADOR,
				ALGORITMO_REEMPLAZO,
				PUNTO_MONTAJE,
				NOMBRE_INSTANCIA,
				INTERVALO_DUMP
				);
	fflush(stdout);
}

int ceilDivision(int lengthValue) {
	double cantidadEntradas;
	cantidadEntradas = (lengthValue + TAMANIO_ENTRADA - 1) / TAMANIO_ENTRADA;
	return cantidadEntradas;

}

int getFirstIndex (int entradasValue){
	int i;
	for (i=0;  i< CANT_ENTRADA; i++) {
		if(!strcmp(tabla_entradas[i],"NaN") &&  tabla_entradas[entradasValue-1]){
			int aux;
			bool cumple=true;
			//evaluo valores intermedios entre el inicio y el supuesto final (entradasValue-1)
			for(aux=i+1; aux< entradasValue; aux++){
				if(strcmp(tabla_entradas[aux],"NaN")){
					cumple=false;
					break;
				}
			}
			if(cumple)
				return i;
		}
	}
	return -1;
}
void verificarPuntoMontaje(){

	DIR* dir = opendir(PUNTO_MONTAJE);
	if (dir)
	{
	    /* Directory exists. */
		/* El directorio existe. Hay que recorrer todos sus archivos y por cada uno de ellos
		 *  verificar el nombre del archivo(clave) y lo que haya dentro es el value(valor)
		 * Despues hay que añadirlo a la tabla de entradas y mandarle un mensaje a COORDINADOR con las entradas que tiene */
	    closedir(dir);
	}
	else if (ENOENT == errno)
	{
		//el directorio no existe
//		log_info(vg_logger,"No existe el punto de montaje, se crea el directorio");
		mkdir(PUNTO_MONTAJE,0700);
	}
	else
	{
		log_error(vg_logger, "Se detectó el siguiente error al abrir el directorio: %s", strerror(errno));
	    printf("Fallo el opendir \n");
	    fflush(stdout);
	}
}
void crearArchivo(char*key,char*value){
	char *ruta=malloc(strlen(PUNTO_MONTAJE)+strlen("/")+strlen(key)+1);
	strcpy(ruta,PUNTO_MONTAJE);
	strcat(ruta,"/");
	strcat(ruta,key);
	FILE* f= fopen(ruta,"w");
	fwrite(value,1,sizeof(value)+1,f);
	fclose(f);
	free(ruta);
}


//No se puso en el SWITCH debido a que es PROPIO DE LA INSTANCIA! No depende del COORDINADOR.

void dump(){
	while(1){
		usleep(20000000);//20 segundos
			//Hago un for con la cantidad de entradas administrativas para saber claves que tengo
			int i,j;
			for (i=0;  i< list_size(entradas_administrativa); i++) {

				/* Agarro las claves con list_get */
				t_Entrada* actual = (t_Entrada*)list_get(entradas_administrativa, i);

				/* Creo el directorio dinámico de la clave */
				char* directorio_actual = malloc(strlen(PUNTO_MONTAJE) + strlen(actual->clave) + 2);
				strcpy(directorio_actual, PUNTO_MONTAJE);
				strcpy(directorio_actual+strlen(PUNTO_MONTAJE),actual->clave);

				//Valor que va a tener la clave dentro de la tabla de entradas
				char* valor=malloc(actual->tamanio);
				int tamanioPegado = 0;

				//Recorro tabla entradas para obtener clave completa
				for (j = actual->index; j < (actual->index + actual->entradasOcupadas);j++) {
					//Pregunto si tiene una sola entrada o utiliza más
					if((actual->index + actual->entradasOcupadas) -1 == j){
						strcpy(valor + tamanioPegado, tabla_entradas[j]);
					}else{
						strcpy(valor + tamanioPegado, tabla_entradas[j]);
						tamanioPegado += TAMANIO_ENTRADA;
					}
				}

				//Creo archivo con dirección dinámica (directorio)
				FILE* file_a_crear = fopen(directorio_actual,"w+");

				//Escribo la clave en el archivo
				fwrite(valor,actual->tamanio,sizeof(char),file_a_crear);

				//Libero memoria
				free(valor);
				fclose(file_a_crear);
			}

	}

}



int main(void) {
	obtenerValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
	verificarPuntoMontaje();
	entradas_administrativa=list_create();
	//socket que maneja la conexion con coordinador
	int socketCoordinador=ConectarAServidor(PUERTO_COORDINADOR, IP_COORDINADOR, COORDINADOR, INSTANCIA, RecibirHandshake);
	//dump();
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteCliente(socketCoordinador, INSTANCIA, &paquete)>0){
		datos=paquete.Payload;
		switch(paquete.header.tipoMensaje){
			case SOLICITUDNOMBRE:{
				EnviarDatosTipo(socketCoordinador,INSTANCIA,NOMBRE_INSTANCIA,strlen(NOMBRE_INSTANCIA)+1,IDENTIFICACIONINSTANCIA);
			}
			break;
			case STOREINST:{
				char *key = malloc(strlen(datos)+1);
				strcpy(key,datos);
				t_Entrada *esperada = list_find(entradas_administrativa,LAMBDA(int _(t_Entrada *elemento) {  return !strcmp(key,elemento->clave);}));
				char *valueReturn = malloc(esperada->tamanio+1);
				for (int var = esperada->index; var < esperada->index+esperada->entradasOcupadas; var++) {
					if((esperada->index+esperada->entradasOcupadas)-1 == var){
						strcpy(valueReturn,tabla_entradas[var]);
						valueReturn+=strlen(tabla_entradas[var]);
						break;
					}
					strncpy(valueReturn,tabla_entradas[var],TAMANIO_ENTRADA);
					valueReturn+=TAMANIO_ENTRADA;
				}
				valueReturn-=esperada->tamanio;
				crearArchivo(key,valueReturn);
				EnviarDatosTipo(socketCoordinador,INSTANCIA,key,strlen(key)+1,STOREOK);
				free(key);
				free(valueReturn);
			}
			break;
			case SETINST:{
				char*key=malloc(strlen(datos)+1);
				strcpy(key,datos);
				datos+=strlen(datos)+1;
				char* value=malloc(strlen(datos)+1);
				strcpy(value,datos);
				t_Entrada *nueva=malloc(sizeof(t_Entrada));
				nueva->clave=malloc(strlen(key)+1);
				strcpy(nueva->clave,key);
				nueva->entradasOcupadas = ceilDivision(strlen(value));
				nueva->tamanio = strlen(value);
				nueva->index = getFirstIndex(nueva->entradasOcupadas);
				list_add(entradas_administrativa,nueva);  //
				int i;
				char *valueAux=malloc(strlen(value)+1);
				strcpy(valueAux,value);
				for(i=nueva->index;i<(nueva->index+nueva->entradasOcupadas);i++){
					if((nueva->index+nueva->entradasOcupadas)-1 == i){
						strcpy(tabla_entradas[i],valueAux);
						break;
					}
					strncpy(tabla_entradas[i],valueAux,TAMANIO_ENTRADA);
					valueAux+=TAMANIO_ENTRADA;
				}
				EnviarDatosTipo(socketCoordinador,INSTANCIA,key,strlen(key)+1,SETOK);
				free(key);
				free(value);

			}
			break;
			case GETENTRADAS:{
				TAMANIO_ENTRADA=*((int*)datos);
				datos+=sizeof(int);
				CANT_ENTRADA=*((int*)datos);
				datos+=sizeof(int);
				tabla_entradas=malloc(CANT_ENTRADA*sizeof(char*)); //CANT_ENTRADA * TAMANIO_ENTRADA
				int i;
				for(i=0;i<CANT_ENTRADA;i++){
					tabla_entradas[i]=malloc(TAMANIO_ENTRADA);
					strcpy(tabla_entradas[i],"NaN");
				}
			}
			break;
		}
		if (paquete.Payload != NULL){
			free(paquete.Payload);
		}
	}
	return EXIT_SUCCESS;
}
























