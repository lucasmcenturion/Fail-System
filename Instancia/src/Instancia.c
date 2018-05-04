#include "sockets.h"

char *IP_COORDINADOR, *ALGORITMO_REEMPLAZO, *PUNTO_MONTAJE, *NOMBRE_INSTANCIA;
int PUERTO_COORDINADOR, INTERVALO_DUMP, TAMANIO_ENTRADA, CANT_ENTRADA;
char **tabla_entradas;
t_list *entradas_administrativa;

void obtenerValoresArchivoConfiguracion() {
	t_config* arch = config_create("/home/utnso/workspace/tp-2018-1c-Fail-system/Instancia/instanciaCFG.txt");
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
			for(aux=i+1; i< entradasValue; aux++){
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

int main(void) {
	obtenerValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
	entradas_administrativa=list_create();
	//socket que maneja la conexion con coordinador
	int socketCoordinador=ConectarAServidor(PUERTO_COORDINADOR, IP_COORDINADOR, COORDINADOR, INSTANCIA, RecibirHandshake);
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteCliente(socketCoordinador, INSTANCIA, &paquete)>0){
		datos=paquete.Payload;
		switch(paquete.header.tipoMensaje){
			case SOLICITUDNOMBRE:{
				EnviarDatosTipo(socketCoordinador,INSTANCIA,NOMBRE_INSTANCIA,strlen(NOMBRE_INSTANCIA)+1,IDENTIFICACIONINSTANCIA);
			}
			break;
			case GET:{
				int tamanioKey = *((int*)datos);
				datos+=sizeof(int);
				char *key = malloc(tamanioKey);
				strcpy(key,datos);
			}
			break;
			case SET:{
				int tamanioKey = *((int*)datos);
				datos+=sizeof(int);
				char *key = malloc(tamanioKey);
				strcpy(key,datos);
				datos+=strlen(tamanioKey);
				int tamanioValue = *((int*)datos);
				datos +=sizeof(int);
				char *value = malloc(tamanioValue);
				strcpy(value,datos);

				t_Entrada *nueva=malloc(sizeof(t_Entrada));
				nueva->clave=malloc(tamanioKey);
				strcpy(nueva->clave,key);
				nueva->entradasOcupadas = ceilDivision(strlen(value));
				nueva->tamanio = strlen(value);
				nueva->index = getFirstIndex(nueva->tamanio);
				list_add(entradas_administrativa,nueva);
				int i;
				/*for(i=nueva->index;i<nueva->index+nueva->tamanio;i++){
					strcpy(tabla_entradas[])
				}*/
			}
			break;
			case GETENTRADAS:{
				TAMANIO_ENTRADA=*((int*)datos);
				datos+=sizeof(int);
				CANT_ENTRADA=*((int*)datos);
				datos+=sizeof(int);
				tabla_entradas=malloc(CANT_ENTRADA*sizeof(char*));
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
