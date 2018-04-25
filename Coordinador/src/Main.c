#include "Coordinador.h"
#include "sockets.h"

t_list* listaHilos;

int main(){

	obtenerValoresArchivoConfiguracion();

	imprimirArchivoConfiguracion();

	ServidorConcurrente(IP, PUERTO, COORDINADOR, &listaHilos, &end, accion);

	return EXIT_SUCCESS;
}
