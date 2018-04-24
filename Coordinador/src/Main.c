int main(void) {

	obtenerValoresArchivoConfiguracion();

	imprimirArchivoConfiguracion();

	ServidorConcurrente(IP, PUERTO, COORDINADOR, &listaHilos, &end, accion);

	return EXIT_SUCCESS;
}
