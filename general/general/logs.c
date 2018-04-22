/*
 * logs.c
 *
 *  Created on: 9/4/2017
 *      Author: utnso
 */

#include "logs.h"

t_log * vg_logger = NULL;

#define logearError(msg) {log_error(vg_logger,msg);}

// Logger

// Logger
void crearLogger(char* logPath,  char * logMemoNombreArch, bool consolaActiva) {
	vg_logger = log_create(logPath, logMemoNombreArch, consolaActiva, LOG_LEVEL_INFO);
	free(logPath);
//	free(logMemoNombreArch);
}
