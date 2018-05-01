#include "sockets.h"

char *IP_COORDINADOR, *ALGORITMO_REEMPLAZO, *PUNTO_MONTAJE, *NOMBRE_INSTANCIA;
int PUERTO_COORDINADOR, INTERVALO_DUMP;
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

int main(void) {
	obtenerValoresArchivoConfiguracion();
	imprimirArchivoConfiguracion();
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

			}
			break;
			case SET:{

			}
			break;
			case GETENTRADAS:{
				int tamanioEntradas=*((int*)datos);
				datos+=sizeof(int);
				int cantidadEntradas=*((int*)datos);
				datos+=sizeof(int);
				tabla_entradas=malloc(cantidadEntradas*sizeof(char*));
				int i;
				for(i=0;i<cantidadEntradas;i++){
					tabla_entradas[i]=malloc(tamanioEntradas);
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
