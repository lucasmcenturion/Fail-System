#include "serializacion.h"
#include "logs.h"
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>

string leerArchivo(char *path, t_log * log) {
	string result;
	FILE* file = fopen(path, "r");

	if(file == NULL){
	  log_error(log, "Error al abrir el archivo: %s", path);
      exit(EXIT_FAILURE);
	}

	if(fseek(file, 0, SEEK_END) < 0){
		log_error(log, "Error FSEEK");
		exit(EXIT_FAILURE);
	}
	result.tamanio = ftell(file);
	rewind(file);
	result.cadena = malloc(result.tamanio+sizeof(char));
	fread(result.cadena, sizeof(char), result.tamanio, file);
	fclose(file);
	result.cadena[result.tamanio] = '\0';
	return result;
}

string leerArchivo2(char *path) {
 	string result;
 	FILE* file = fopen(path, "r");
 	fseek(file, 0, SEEK_END);
 	result.tamanio = ftell(file);
 	rewind(file);
 	result.cadena = malloc(result.tamanio);
 	fread(result.cadena, sizeof(char), result.tamanio, file);
 	fclose(file);
 	return result;
 }

