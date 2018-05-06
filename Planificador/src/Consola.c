#include "Consola.h"

bool planificacion_detenida;

void PausarContinuar(){
	planificacion_detenida = !planificacion_detenida;
	planificacion_detenida ? printf("La planificación se detuvo.\n") : printf("La planificación se reanudó.\n");
}

void Bloquear(){

}

void Desbloquear(){

}

void Listar(){

}

void Kill(){

}

void Status(){

}

void Deadlock(){

}
void consola() {
	char * linea;
	while (true) {
		linea = readline(">> ");
		if (linea)
			add_history(linea);
		if (!strcmp(linea, "pausar/continuar")) {
			PausarContinuar();
		}
		else if (!strncmp(linea, "bloquear ", 9)) {
			char **array_input = string_split(linea, " ");
			printf("Se bloqueó el proceso ESI de id %s, en la cola del recurso %s.\n", array_input[2], array_input[1]);
			Bloquear();
			string_iterate_lines(array_input,free);
			free(array_input);
		}
		else if (!strncmp(linea, "desbloquear ", 12)) {
			char **array_input = string_split(linea, " ");
			printf("Se desbloqueó el primer proceso ESI en la cola del recurso %s.\n", array_input[1]);
			Desbloquear();
			string_iterate_lines(array_input,free);
			free(array_input);
		}
		else if (!strncmp(linea, "listar ", 7)) {
			char **array_input = string_split(linea, " ");
			printf("Se listan los procesos bloqueados esperando el recurso %s.\n", array_input[1]);
			Listar();
			string_iterate_lines(array_input,free);
		free(array_input);
		}

		else if (!strncmp(linea, "kill ", 5)) {
			char **array_input = string_split(linea, " ");
			printf("Se finaliza el proceso de id %s.\n", array_input[1]);
			Kill();
			string_iterate_lines(array_input,free);
			free(array_input);
		}
		else if (!strncmp(linea, "status ", 7)) {
			char **array_input = string_split(linea, " ");
			printf("Se muestra el estado de la clave %s.\n", array_input[1]);
			Status();
			string_iterate_lines(array_input,free);
			free(array_input);
		}
		else if (!strcmp(linea, "deadlock")) {
			printf("Se muestra deadlocks del sistema.\n");
			Deadlock();
		}
		else if (!strcmp(linea, "exit")) {
			printf("Salió\n");
			free(linea);
			break;
		} else
			printf("No se conoce el comando\n");
		free(linea);
	}
}
