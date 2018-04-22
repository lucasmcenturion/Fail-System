#include "archivoConfig.h"
#define manejarError(msg) {perror(msg); abort();}

#define LONGPATH 200
#define FAIL -1

void leerConfig(char * configPath) {
	leerArchivoDeConfiguracion(configPath);
//free(configPath);
	if (comprobarValoresBienSeteados()) {
		log_info(vg_logger, "Archivo de configuracion leido correctamente");
	}
}

void leerArchivoDeConfiguracion(char * configPath) {
	t_config * archivoConfig;

	if (verificarExistenciaDeArchivo(configPath) == FAIL)
		manejarError("[ERROR] Archivo de configuracion no encontrado");

	archivoConfig = config_create(configPath);
	setearValores(archivoConfig);
	config_destroy(archivoConfig);
}

int verificarExistenciaDeArchivo(char* rutaArchivoConfig) {
	FILE * archivoConfig = fopen(rutaArchivoConfig, "r");
	if (archivoConfig != NULL) {
		fclose(archivoConfig);
		return 1;
	}
	return FAIL;
}

char * ingresarRuta(char * deQue) {
	char * path = malloc(sizeof(char) * 100);
	printf("Ingrese la ruta del archivo de %s: ", deQue);
	scanf("%s", path);
	free(deQue);
	return path;
}

bool validarSetting(void* valor, const char* nombrePropiedad) {
	bool hasValue = valor;
	if (!hasValue) {
		char* message = string_new();
		string_append_with_format(&message,
				"Archivo configuracion: %s mal configurado",
				nombrePropiedad);
		log_error(vg_logger, message);
		free(message);
	}

	return hasValue;
}

