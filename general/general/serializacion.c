#include "serializacion.h"
#include <stdlib.h>

void empaquetar(package *paquete, void* datos, int tamanio) {
	memcpy(paquete->buffer + paquete->desplazamiento, datos, tamanio);
	paquete->desplazamiento += tamanio;
}

void desempaquetar(package *paquete, void* dato, int tamanio) {
	memcpy(dato, paquete->buffer + paquete->desplazamiento, tamanio);
	paquete->desplazamiento += tamanio;
}

void empaquetarEntero(package *paquete, int *entero) {
	empaquetar(paquete, entero, sizeof(int));

}

void desempaquetarEntero(package *paquete, int *entero) {
	desempaquetar(paquete, entero, sizeof(int));
}

void empaquetarCadena(package *paquete, string *cadena) {
	empaquetarEntero(paquete, &cadena->tamanio);
	empaquetar(paquete, cadena->cadena, cadena->tamanio);
}

void desempaquetarCadena(package *paquete, string *cadena) {
	desempaquetarEntero(paquete, &cadena->tamanio);
	cadena->cadena = malloc((cadena->tamanio + 1) * sizeof(char));
	desempaquetar(paquete, cadena->cadena, cadena->tamanio);
	cadena->cadena[cadena->tamanio] = '\0';
}

void empaquetarNodoSolicitudReduccionGlobal(package *paquete,
		t_nodoSolicitudReduccionGlobal *nodo) {
	empaquetarCadena(paquete, &nodo->idNodo);
	empaquetarEntero(paquete, &nodo->direccion.puerto);
	empaquetarCadena(paquete, &nodo->direccion.ip);
	empaquetarCadena(paquete, &nodo->archivoTempReduccionLocal);
}

int getNodesTotalSize(t_list* list, int (*closure)(void*)) {
	int size = 0;

	void increaseSize(void* element) {
		size += closure(element);
	}

	list_iterate(list, increaseSize);
	return size;
}

int getSizeOfNodoSolicitudReduccionGlobal(t_nodoSolicitudReduccionGlobal *nodo) {
	int size = 0;
	size += getStringSize(&nodo->idNodo);
	size += sizeof(int);
	size += getStringSize(&nodo->direccion.ip);
	size += getStringSize(&nodo->archivoTempReduccionLocal);
	return size;
}

void empaquetarNodo(package* paquete, nodo * nodo) {
	empaquetarCadena(paquete, &nodo->idNodo);
	empaquetarEntero(paquete, &nodo->numeroBloque);
	empaquetarEntero(paquete, &nodo->espacioEnDisco);
	empaquetarEntero(paquete, &nodo->direccion.puerto);
	empaquetarCadena(paquete, &nodo->direccion.ip);
}

void empaquetarListaNodoRespuestaTrasnformacion(package *paquete,
		t_list * lista) {
	//le agrego al buffer la cantidad de nodos
	int cantidadNodos = list_size(lista);
	empaquetarEntero(paquete, &cantidadNodos);
	int i;
	for (i = 0; i < list_size(lista); i++) {
		t_nodoRespuestaTransformacion * nodo = list_get(lista, i);
		empaquetarNodo(paquete, &nodo->nodo);
		empaquetarCadena(paquete, &nodo->archivoTemporal);
	}
}

void desempaquetarNodo(package* paquete, nodo * nodo) {
	desempaquetarCadena(paquete, &nodo->idNodo);
	desempaquetarEntero(paquete, &nodo->numeroBloque);
	desempaquetarEntero(paquete, &nodo->espacioEnDisco);
	desempaquetarEntero(paquete, &nodo->direccion.puerto);
	desempaquetarCadena(paquete, &nodo->direccion.ip);
}

void desempaquetarListaNodoRespuestaTrasnformacion(package *paquete,
		t_list * lista) {
	int tamanioLista, i;
	desempaquetarEntero(paquete, &tamanioLista);
	for (i = 0; i < tamanioLista; i++) {
		t_nodoRespuestaTransformacion * nodo = malloc(
				sizeof(t_nodoRespuestaTransformacion));
		desempaquetarNodo(paquete, &nodo->nodo);
		desempaquetarCadena(paquete, &nodo->archivoTemporal);
		list_add(lista, nodo);
	}
}

void empaquetarListaNodoRespuestaBloquesArchivo(package *paquete,
		t_list * lista) {
	//le agrego al buffer la cantidad de nodos
	int cantidadNodos = list_size(lista);
	empaquetarEntero(paquete, &cantidadNodos);
	int i;
	for (i = 0; i < list_size(lista); i++) {
		t_nodoRespuestaBloquesArchivo * nodo =
				(t_nodoRespuestaBloquesArchivo *) list_get(lista, i);
		empaquetarEntero(paquete, &nodo->bloqueArchivo);
		empaquetarNodo(paquete, &nodo->nodoOriginalCopia.nodoOriginal);
		empaquetarNodo(paquete, &nodo->nodoOriginalCopia.nodoCopia);
	}
}

void desempaquetarListaNodoRespuestaBloquesArchivo(package *paquete,
		t_list * lista) {
	int tamanioLista, i;
	desempaquetarEntero(paquete, &tamanioLista);
	for (i = 0; i < tamanioLista; i++) {
		t_nodoRespuestaBloquesArchivo * nodo = malloc(
				sizeof(t_nodoRespuestaBloquesArchivo));
		desempaquetarEntero(paquete, &nodo->bloqueArchivo);
		desempaquetarNodo(paquete, &nodo->nodoOriginalCopia.nodoOriginal);
		desempaquetarNodo(paquete, &nodo->nodoOriginalCopia.nodoCopia);
		list_add(lista, nodo);
	}
}

void empaquetarListaNodoSolicitudReduccionGlobal(package *paquete,
		t_list * lista) {
	int cantidadNodos = list_size(lista);
	empaquetarEntero(paquete, &cantidadNodos);
	int i;
	for (i = 0; i < list_size(lista); i++) {
		t_nodoSolicitudReduccionGlobal * nodo =
				(t_nodoSolicitudReduccionGlobal *) list_get(lista, i);
		empaquetarNodoSolicitudReduccionGlobal(paquete, nodo);
	}
}

t_nodoSolicitudReduccionGlobal* desempaquetarNodoSolicitudReduccionGlobal(
		package* paquete) {
	t_nodoSolicitudReduccionGlobal* nodo = malloc(
			sizeof(t_nodoSolicitudReduccionGlobal));
	desempaquetarCadena(paquete, &nodo->idNodo);
	desempaquetarEntero(paquete, &nodo->direccion.puerto);
	desempaquetarCadena(paquete, &nodo->direccion.ip);
	desempaquetarCadena(paquete, &nodo->archivoTempReduccionLocal);
	return nodo;
}

void desempaquetarListaNodoSolicitudReduccionGlobal(package *paquete,
		t_list * lista) {
	int tamanioLista, i;
	desempaquetarEntero(paquete, &tamanioLista);
	for (i = 0; i < tamanioLista; i++) {
		t_nodoSolicitudReduccionGlobal* nodo =
				desempaquetarNodoSolicitudReduccionGlobal(paquete);
		list_add(lista, nodo);
	}
}

void empaquetarListaNodoRespuestaReduccionLocal(package *paquete,
		t_list * lista) {
	int cantidadNodos = list_size(lista);
	empaquetarEntero(paquete, &cantidadNodos);
	int i;
	for (i = 0; i < list_size(lista); i++) {
		t_nodoRespuestaReduccionLocal * nodo =
				(t_nodoRespuestaReduccionLocal *) list_get(lista, i);
		empaquetarCadena(paquete, &nodo->archivoTempTransformacion);

	}
}

void desempaquetarListaNodoRespuestaReduccionLocal(package *paquete,
		t_list * lista) {
	int tamanioLista, i;
	desempaquetarEntero(paquete, &tamanioLista);
	for (i = 0; i < tamanioLista; i++) {
		t_nodoRespuestaReduccionLocal * nodo = malloc(
				sizeof(t_nodoRespuestaReduccionLocal));
		desempaquetarCadena(paquete, &nodo->archivoTempTransformacion);
		list_add(lista, nodo);
	}
}

package* crearPaqueteDeEnvio(int tipoMensaje, int tamanio) {
	package *paquete = malloc(sizeof(package));
	paquete->tamanio = tamanio + sizeof(int) * 2;
	paquete->desplazamiento = 0;
	paquete->buffer = malloc(paquete->tamanio);
	empaquetarEntero(paquete, &tipoMensaje);
	empaquetarEntero(paquete, &tamanio);
	return paquete;
}

int tamanioDeStringsPorTipoNodo(int tipoNodo, t_list * lista) {
	int tamanio;
	/// abrirlo por tipo de NODO
	switch (tipoNodo) {
	case tnodoRespuestaTransformacion:
		tamanio = stringsNodoRespuestaTrasnformacion(lista);
		break;
	case tnodoRespuestaBloquesArchivo:
		tamanio = stringsNodoRespuestaBloquesArchivo(lista);
		break;
	case tnodoSolicitudReduccionGlobal:
		tamanio = stringsNodoSolicitudReduccionGlobal(lista);
		break;
	case tnodoRespuestaReduccionLocal:
		tamanio = stringsNodoRespuestaReduccionLocal(lista);
		break;
	default:
		break;
	}

	return tamanio;
}
int stringsNodoRespuestaTrasnformacion(t_list * lista) {
	int tamanio = 0;
	int i;
	for (i = 0; i < list_size(lista); i++) {
		t_nodoRespuestaTransformacion * nodo =
				(t_nodoRespuestaTransformacion *) list_get(lista, i);
		tamanio += getStringSize(&nodo->nodo.idNodo);
		tamanio += getStringSize(&nodo->nodo.direccion.ip);
		tamanio += getStringSize(&nodo->archivoTemporal);
	}
	return tamanio * sizeof(char);
}

int stringsNodoRespuestaBloquesArchivo(t_list * lista) {
	int tamanio = 0;
	int i;
	for (i = 0; i < list_size(lista); i++) {
		t_nodoRespuestaBloquesArchivo * nodo =
				(t_nodoRespuestaBloquesArchivo *) list_get(lista, i);
		tamanio += getStringSize(&nodo->nodoOriginalCopia.nodoOriginal.idNodo);
		tamanio += getStringSize(
				&nodo->nodoOriginalCopia.nodoOriginal.direccion.ip);
		tamanio += getStringSize(&nodo->nodoOriginalCopia.nodoCopia.idNodo);
		tamanio += getStringSize(
				&nodo->nodoOriginalCopia.nodoCopia.direccion.ip);
	}
	return tamanio;
}

int stringsNodoSolicitudReduccionGlobal(t_list * lista) {
	int tamanio = 0;
	int i;
	for (i = 0; i < list_size(lista); i++) {
		t_nodoSolicitudReduccionGlobal * nodo =
				(t_nodoSolicitudReduccionGlobal *) list_get(lista, i);
		tamanio += getStringSize(&nodo->idNodo);
		tamanio += getStringSize(&nodo->direccion.ip);
		tamanio += getStringSize(&nodo->archivoTempReduccionLocal);
	}
	return tamanio;
}

int stringsNodoRespuestaReduccionLocal(t_list * lista) {
	int tamanio = 0;
	int i;
	for (i = 0; i < list_size(lista); i++) {
		t_nodoRespuestaReduccionLocal * nodo =
				(t_nodoRespuestaReduccionLocal *) list_get(lista, i);
		tamanio += getStringSize(&nodo->archivoTempTransformacion);

	}
	return tamanio * sizeof(char);
}

package* serializarRespuestaTransformacion(void* data) {

	respuestaTransformacion * respuesta = (respuestaTransformacion*) data;
	//Calculo el tamanio de los char*(string) de los nodo de las lista para despues armar el paquete
	int tamanioStrings = tamanioDeStringsPorTipoNodo(
			tnodoRespuestaTransformacion, respuesta->nodos);
	int tamanioStructura = list_size(respuesta->nodos) * (sizeof(int) * 3);
	int cantidadNodos = sizeof(int);
	int tamanioTotal = tamanioStrings + tamanioStructura + cantidadNodos;
	package* paquete = crearPaqueteDeEnvio(tRespuestaTransformacion,
			tamanioTotal);
	//Empaqueto los componentes
	empaquetarListaNodoRespuestaTrasnformacion(paquete, respuesta->nodos);
	return paquete;
}

respuestaTransformacion* deserializarRespuestaTransformacion(package* paquete) {
	respuestaTransformacion *respuesta = malloc(
			sizeof(respuestaTransformacion));
	respuesta->nodos = list_create();
	desempaquetarListaNodoRespuestaTrasnformacion(paquete, respuesta->nodos);
	return respuesta;
}

package* serializarRespuestaBloqueArchivo(void* data) {
	respuestaBloquesArchivo * respuesta = (respuestaBloquesArchivo*) data;
	int tamanioStringPrincipal = getStringSize(&respuesta->nombreArchivo);
	//Calculo el tamanio de los char*(string) de los nodo de las lista para despues armar el paquete
	int tamanioStrings = tamanioDeStringsPorTipoNodo(
			tnodoRespuestaBloquesArchivo, respuesta->nodos);
	int tamanioStructura = list_size(respuesta->nodos) * (sizeof(int) * 7);
	int cantidadNodos = sizeof(int);
	int tamanioEstado = sizeof(int);
	int tamanioTotal = tamanioStrings + tamanioStructura + cantidadNodos
			+ tamanioEstado + tamanioStringPrincipal;
	package* paquete = crearPaqueteDeEnvio(tRespuestaBloquesArchivo,
			tamanioTotal);
	//Empaqueto los componentes
	empaquetarEntero(paquete, &respuesta->estado);
	empaquetarCadena(paquete, &respuesta->nombreArchivo);
	empaquetarListaNodoRespuestaBloquesArchivo(paquete, respuesta->nodos);
	return paquete;
}
respuestaBloquesArchivo* deserializarRespuestaBloqueArchivo(package* paquete) {
	respuestaBloquesArchivo *respuesta = malloc(
			sizeof(respuestaBloquesArchivo));
	respuesta->nodos = list_create();
	desempaquetarEntero(paquete, &respuesta->estado);
	desempaquetarCadena(paquete, &respuesta->nombreArchivo);
	desempaquetarListaNodoRespuestaBloquesArchivo(paquete, respuesta->nodos);
	return respuesta;
}

package* serializarSolicitudReduccionGlobal(void* data) {
	solicitudReduccionGlobal * solicitud = (solicitudReduccionGlobal*) data;
	int tamanioStringEncargado = getStringSize(&solicitud->encargado);
	int tamanioStringArchivoTempRG = getStringSize(
			&solicitud->archivoTempReduccionGlobal);
	//Calculo el tamanio de los char*(string) de los nodo de las lista para despues armar el paquete
	int tamanioStrings = tamanioDeStringsPorTipoNodo(
			tnodoSolicitudReduccionGlobal, solicitud->nodos);
	int tamanioPuertosNodo = list_size(solicitud->nodos) * sizeof(int);
	int cantidadNodos = sizeof(int);
	int tamanioTotal = tamanioStringEncargado + tamanioStringArchivoTempRG
			+ tamanioStrings + tamanioPuertosNodo + cantidadNodos;
	package* paquete = crearPaqueteDeEnvio(tSolicitudReduccionGlobal,
			tamanioTotal);
	//Empaqueto los componentes
	empaquetarCadena(paquete, &solicitud->encargado);
	empaquetarCadena(paquete, &solicitud->archivoTempReduccionGlobal);
	empaquetarListaNodoSolicitudReduccionGlobal(paquete, solicitud->nodos);
	return paquete;
}

solicitudReduccionGlobal* deserializarSolicitudReduccionGlobal(package* paquete) {
	solicitudReduccionGlobal *solicitud = malloc(
			sizeof(solicitudReduccionGlobal));
	solicitud->nodos = list_create();
	desempaquetarCadena(paquete, &solicitud->encargado);
	desempaquetarCadena(paquete, &solicitud->archivoTempReduccionGlobal);
	desempaquetarListaNodoSolicitudReduccionGlobal(paquete, solicitud->nodos);
	return solicitud;
}

package* serializarRespuestaReduccionLocal(void* data) {
	respuestaReduccionLocal * respuesta = (respuestaReduccionLocal*) data;
	//Calculo el tamanio de los char*(string) de los nodo de las lista para despues armar el paquete
	int tamanioStrings = getStringSize(&respuesta->idNodo)
			+ getStringSize(&respuesta->direccion.ip)
			+ getStringSize(&respuesta->archivoTempReduccionLocal);
	int tamanioStringsLista = tamanioDeStringsPorTipoNodo(
			tnodoRespuestaReduccionLocal, respuesta->nodos);
	int cantidadNodos = sizeof(int);
	int tamanioPuerto = sizeof(int);
	int tamanioTotal = tamanioStrings + tamanioStringsLista + cantidadNodos
			+ tamanioPuerto;
	package* paquete = crearPaqueteDeEnvio(tRespuestaReduccionLocal,
			tamanioTotal);
	empaquetarCadena(paquete, &respuesta->idNodo);
	empaquetarCadena(paquete, &respuesta->direccion.ip);
	empaquetarEntero(paquete, &respuesta->direccion.puerto);
	empaquetarCadena(paquete, &respuesta->archivoTempReduccionLocal);
	//Empaqueto los componentes
	empaquetarListaNodoRespuestaReduccionLocal(paquete, respuesta->nodos);
	return paquete;
}

respuestaReduccionLocal* deserializarRespuestaReduccionLocal(package* paquete) {
	respuestaReduccionLocal *respuesta = malloc(
			sizeof(respuestaReduccionLocal));
	respuesta->nodos = list_create();
	desempaquetarCadena(paquete, &respuesta->idNodo);
	desempaquetarCadena(paquete, &respuesta->direccion.ip);
	desempaquetarEntero(paquete, &respuesta->direccion.puerto);
	desempaquetarCadena(paquete, &respuesta->archivoTempReduccionLocal);
	desempaquetarListaNodoRespuestaReduccionLocal(paquete, respuesta->nodos);
	return respuesta;
}

package* serializarTransformacion(void* data) {
	transformacion* transf = (transformacion*) data;
	int tamanioPathTemporal = getStringSize(&transf->pathTemporal);
	int tamanioTotal = sizeof(int) + sizeof(int) + tamanioPathTemporal;

	package* paquete = crearPaqueteDeEnvio(tTransformacion, tamanioTotal);
	empaquetarEntero(paquete, &transf->numeroDeBloque);
	empaquetarEntero(paquete, &transf->cantidadDeBytes);
	empaquetarCadena(paquete, &transf->pathTemporal);
	return paquete;
}

package* serializarReduccion(void* data) {
	reduccionLocal* reduccion = (reduccionLocal*) data;
	int i = 0;
	int tamanio_cantidad_elementos = sizeof(int);
	int cantidad_elementos = list_size(reduccion->archivosTemporales);
	int tamanio_lista = 0;
	int tamanio_pathtemp = getStringSize(&reduccion->pathTemporal);
	for (i = 0; i < list_size(reduccion->archivosTemporales); i++) {
		tamanio_lista += getStringSize(
				(string*) list_get(reduccion->archivosTemporales, i));
	}
	int tamanioTotal = tamanio_pathtemp + tamanio_cantidad_elementos + tamanio_lista;
	package* paquete = crearPaqueteDeEnvio(tReduccionLocal, tamanioTotal);

	empaquetarCadena(paquete, &reduccion->pathTemporal);
	empaquetarEntero(paquete, &cantidad_elementos);
	void empaquetarLista(string* elemento) {
		empaquetarCadena(paquete, elemento);
	}

	list_iterate(reduccion->archivosTemporales, empaquetarLista);
	return paquete;
}

package* serializarReduccionGlobal(void* data) {
	reduccionGlobal* reduccion = (reduccionGlobal*) data;
	int tamanioTotal = 0;
	tamanioTotal += getStringSize(&reduccion->pathReduccionGlobal);
	tamanioTotal += getSizeOfNodoSolicitudReduccionGlobal(reduccion->encargado);
	tamanioTotal += sizeof(int);
	tamanioTotal += getNodesTotalSize(reduccion->workers,
			&getSizeOfNodoSolicitudReduccionGlobal);

	package* paquete = crearPaqueteDeEnvio(tReduccionGlobal, tamanioTotal);
	empaquetarCadena(paquete, &reduccion->pathReduccionGlobal);
	empaquetarNodoSolicitudReduccionGlobal(paquete, reduccion->encargado);
	int workersQuantity = list_size(reduccion->workers);
	empaquetarEntero(paquete, &workersQuantity);

	void empaquetarWorker(t_nodoSolicitudReduccionGlobal *worker) {
		empaquetarNodoSolicitudReduccionGlobal(paquete, worker);
	}

	list_iterate(reduccion->workers, empaquetarWorker);
	return paquete;
}

transformacion* deserializarTransformacion(package* paquete) {
	transformacion *transf = malloc(sizeof(transformacion));
	desempaquetarEntero(paquete, &transf->numeroDeBloque);
	desempaquetarEntero(paquete, &transf->cantidadDeBytes);
	desempaquetarCadena(paquete, &transf->pathTemporal);
	return transf;
}

transformacion* deserializarReduccion(package* paquete) {
	reduccionLocal* reduccion = malloc(sizeof(reduccionLocal));
	reduccion->archivosTemporales = list_create();
	desempaquetarCadena(paquete, &reduccion->pathTemporal);
	//desempaquetarListaArchivosReduccion(paquete, reduccion->archivosTemporales);
	int tamanioLista, i;
	desempaquetarEntero(paquete, &tamanioLista);
	for (i = 0; i < tamanioLista; i++) {
		string* elemento = malloc(sizeof(string));
		desempaquetarCadena(paquete, elemento);
		list_add(reduccion->archivosTemporales, elemento);
	}

	return reduccion;
}

reduccionGlobal* deserializarReduccionGlobal(package* paquete) {
	reduccionGlobal* reduccion = malloc(sizeof(reduccionGlobal));
	reduccion->workers = list_create();
	desempaquetarCadena(paquete, &reduccion->pathReduccionGlobal);
	reduccion->encargado = desempaquetarNodoSolicitudReduccionGlobal(paquete);
	desempaquetarListaNodoSolicitudReduccionGlobal(paquete, reduccion->workers);
	return reduccion;
}

void desempaquetarListaArchivosReduccion(package *paquete, t_list * lista) {
	int tamanioLista, i;
	desempaquetarEntero(paquete, &tamanioLista);
	for (i = 0; i < tamanioLista; i++) {
		string* elemento = malloc(sizeof(string));
		desempaquetarCadena(paquete, elemento);
		list_add(lista, elemento);
	}
}

package* serializarEntradaNodo(void* data) {

	t_entradaNodo* entradaNodo = (t_entradaNodo*) data;

	//Calculo el tamanio de los char*(string) de la t_entradaNodo
	int tamanioIdNodo = getStringSize(&entradaNodo->idNodo);
	int tamanioIpNodo = getStringSize(&entradaNodo->ipNodo);

	//Los dos ints hacen referencia a los ints de t_entradaNodo
	int tamanioTotal = sizeof(int) + sizeof(int) + tamanioIdNodo
			+ tamanioIpNodo;

	package* paquete = crearPaqueteDeEnvio(tEntradaNodo, tamanioTotal);

	//Empaqueto los componentes

	empaquetarCadena(paquete, &entradaNodo->idNodo);
	empaquetarCadena(paquete, &entradaNodo->ipNodo);
	empaquetarEntero(paquete, &entradaNodo->puertoWorker);
	empaquetarEntero(paquete, &entradaNodo->tamanioDataBin);

	return paquete;

}

t_entradaNodo* deserializarEntradaNodo(package* paquete) {

	//Reservo memoria para el struct
	t_entradaNodo *entradaNodo = malloc(sizeof(t_entradaNodo));

	desempaquetarCadena(paquete, &entradaNodo->idNodo);
	desempaquetarCadena(paquete, &entradaNodo->ipNodo);
	desempaquetarEntero(paquete, &entradaNodo->puertoWorker);
	desempaquetarEntero(paquete, &entradaNodo->tamanioDataBin);

	return entradaNodo;
}

package* serializarSolicitudTransformacion(void* data) {
	solicitudTransformacion * solicitud = (solicitudTransformacion*) data;
	//Calculo el tamanio de los char*(string) del nombre de Archivo
	int tamanioNombreArchivo = getStringSize(&solicitud->nombreArchivo);
	package* paquete = crearPaqueteDeEnvio(tSolicitudTransformacion,
			tamanioNombreArchivo);
	//Empaqueto los componentes
	empaquetarCadena(paquete, &solicitud->nombreArchivo);
	return paquete;
}

solicitudTransformacion* deserializarSolicitudTransformacion(package* paquete) {
	//Reservo memoria para el struct
	solicitudTransformacion *solicitud = malloc(
			sizeof(solicitudTransformacion));
	desempaquetarCadena(paquete, &solicitud->nombreArchivo);
	return solicitud;
}

package* serializarRespuestaAlmacenadoFinal(void* data) {
	respuestaAlmacenadoFinal * respuesta = (respuestaAlmacenadoFinal*) data;
	//Calculo el tamanio de los char*(string) del nombre de Archivo
	int tamanioStrings = getStringSize(&respuesta->idNodo)
			+ getStringSize(&respuesta->direccion.ip)
			+ getStringSize(&respuesta->archivoTempReduccionGlobal);
	int tamanioTotal = tamanioStrings + sizeof(int);
	package* paquete = crearPaqueteDeEnvio(tRespuestaAlmacenadoFinal,
			tamanioTotal);
	//Empaqueto los componentes
	empaquetarCadena(paquete, &respuesta->idNodo);
	empaquetarEntero(paquete, &respuesta->direccion.puerto);
	empaquetarCadena(paquete, &respuesta->direccion.ip);
	empaquetarCadena(paquete, &respuesta->archivoTempReduccionGlobal);
	return paquete;
}

solicitudTransformacion* deserializarRespuestaAlmacenadoFinal(package* paquete) {
	//Reservo memoria para el struct
	respuestaAlmacenadoFinal * respuesta = malloc(
			sizeof(respuestaAlmacenadoFinal));
	desempaquetarCadena(paquete, &respuesta->idNodo);
	desempaquetarEntero(paquete, &respuesta->direccion.puerto);
	desempaquetarCadena(paquete, &respuesta->direccion.ip);
	desempaquetarCadena(paquete, &respuesta->archivoTempReduccionGlobal);
	return respuesta;
}

package* serializarConfirmacionFinalizacion(void* data) {
	confirmacionFinalizacion * confirmacion = (confirmacionFinalizacion*) data;
	int tamanioId = getStringSize(&confirmacion->idNodo);
	int tamanioTotal = (sizeof(int) * 2) + sizeof(step) + tamanioId;
	package* paquete = crearPaqueteDeEnvio(tConfirmacionFinalizacion,
			tamanioTotal);
	//Empaqueto los componentes
	empaquetarCadena(paquete, &confirmacion->idNodo);
	empaquetarEntero(paquete, &confirmacion->bloque);
	empaquetarEntero(paquete, &confirmacion->tipoOperacion);
	empaquetarEntero(paquete, &confirmacion->estado);
	return paquete;
}

confirmacionFinalizacion* deserializarConfirmacionFinalizacion(package* paquete) {
	//Reservo memoria para el struct
	confirmacionFinalizacion * confirmacion = malloc(
			sizeof(confirmacionFinalizacion));
	desempaquetarCadena(paquete, &confirmacion->idNodo);
	desempaquetarEntero(paquete, &confirmacion->bloque);
	desempaquetarEntero(paquete, &confirmacion->tipoOperacion);
	desempaquetarEntero(paquete, &confirmacion->estado);
	return confirmacion;
}

package* serializarSolicitudBloquesArchivo(void* data) {

	solicitudBloquesArchivo * solicitud = (solicitudBloquesArchivo*) data;
	//Calculo el tamanio de los char*(string) del nombre de Archivo
	int tamanioNombreArchivo = getStringSize(&solicitud->nombreArchivo);
	package* paquete = crearPaqueteDeEnvio(tSolicitudBloquesArchivo,
			tamanioNombreArchivo);
	//Empaqueto los componentes
	empaquetarCadena(paquete, &solicitud->nombreArchivo);
	return paquete;
}

solicitudBloquesArchivo* deserializarSolicitudBloquesArchivo(package* paquete) {
	//Reservo memoria para el struct
	solicitudBloquesArchivo *solicitud = malloc(
			sizeof(solicitudBloquesArchivo));
	desempaquetarCadena(paquete, &solicitud->nombreArchivo);
	return solicitud;
}

package* serializarSolicitudSetBloque(void* data) {
	solicitudSetBloque * solicitud = (solicitudSetBloque*) data;
	int tamanioDatos = getStringSize(&solicitud->datos);
	int tamanioTotal = sizeof(int) + tamanioDatos;
	package* paquete = crearPaqueteDeEnvio(tSolicitudSetBloque, tamanioTotal);
	//Empaqueto los componentes
	empaquetarEntero(paquete, &solicitud->bloque);
	empaquetarCadena(paquete, &solicitud->datos);
	return paquete;
}

solicitudSetBloque* deserializarSolicitudSetBloque(package* paquete) {
	solicitudSetBloque *solicitud = malloc(sizeof(solicitudSetBloque));
	desempaquetarEntero(paquete, &solicitud->bloque);
	desempaquetarCadena(paquete, &solicitud->datos);
	return solicitud;
}

package* serializarSolicitudMetadataGuardarArchivo(void* data) {
	solicitudMetadataGuardarArchivo * solicitud =
			(solicitudMetadataGuardarArchivo*) data;
	int tamanioNombreArchivo = getStringSize(&solicitud->nombreArchivo);
	int tamanioRutaArchivo = getStringSize(&solicitud->ruta);
	int tamanioTotal = sizeof(int) + sizeof(tipoTexto) + sizeof(int)  + tamanioNombreArchivo
			+ tamanioRutaArchivo;
	package* paquete = crearPaqueteDeEnvio(tSolicitudMetadataGuardarArchivo,
			tamanioTotal);
	empaquetarCadena(paquete, &solicitud->ruta);
	empaquetarCadena(paquete, &solicitud->nombreArchivo);
	empaquetarEntero(paquete, &solicitud->tipoTexto);
	empaquetarEntero(paquete, &solicitud->tamanioArchivo);
	empaquetarEntero(paquete, &solicitud->cantBloques);
	return paquete;
}

solicitudMetadataGuardarArchivo * deserializarSolicitudMetadataGuardarArchivo(
		package* paquete) {
	solicitudMetadataGuardarArchivo * solicitud = malloc(
			sizeof(solicitudMetadataGuardarArchivo));
	desempaquetarCadena(paquete, &solicitud->ruta);
	desempaquetarCadena(paquete, &solicitud->nombreArchivo);
	desempaquetarEntero(paquete, &solicitud->tipoTexto);
	desempaquetarEntero(paquete, &solicitud->tamanioArchivo);
	desempaquetarEntero(paquete, &solicitud->cantBloques);
	return solicitud;
}

package* serializarSolicitudGetBloque(void* data) {
	solicitudGetBloque * solicitud = (solicitudGetBloque*) data;
	int tamanioTotal = sizeof(int);
	package* paquete = crearPaqueteDeEnvio(tSolicitudGetBloque, tamanioTotal);
	//Empaqueto los componentes
	empaquetarEntero(paquete, &solicitud->bloque);
	return paquete;
}

solicitudGetBloque * deserializarSolicitudGetBloque(package* paquete) {
	solicitudGetBloque * solicitud = malloc(sizeof(solicitudGetBloque));
	desempaquetarEntero(paquete, &solicitud->bloque);
	return solicitud;
}

package* serializarRespuestaSetBloque(void* data) {
	respuestaSetBloque * respuesta = (respuestaSetBloque*) data;
	int tamanioTotal = sizeof(int);
	package* paquete = crearPaqueteDeEnvio(tRespuestaSetBloque, tamanioTotal);
	//Empaqueto los componentes
	empaquetarEntero(paquete, &respuesta->respuesta);
	return paquete;
}

respuestaSetBloque * deserializarRespuestaSetBloque(package* paquete) {
	respuestaSetBloque * respuesta = malloc(sizeof(respuestaSetBloque));
	desempaquetarEntero(paquete, &respuesta->respuesta);
	return respuesta;
}

package* serializarRespuestaGetBloque(void* data) {
	respuestaGetBloque * respuesta = (respuestaGetBloque*) data;
	int tamanioDatos = getStringSize(&respuesta->datos);
	int tamanioTotal = sizeof(int) + tamanioDatos;
	package* paquete = crearPaqueteDeEnvio(tRespuestaGetBloque, tamanioTotal);
	//Empaqueto los componentes
	empaquetarEntero(paquete, &respuesta->respuesta);
	empaquetarCadena(paquete, &respuesta->datos);
	return paquete;
}

respuestaGetBloque * deserializarRespuestaGetBloque(package* paquete) {
	respuestaGetBloque * respuesta = malloc(sizeof(respuestaGetBloque));
	desempaquetarEntero(paquete, &respuesta->respuesta);
	desempaquetarCadena(paquete, &respuesta->datos);
	return respuesta;
}

package* serializarSolicitudDeFinalizacion(string* errorMessage) {
	package* paquete = crearPaqueteDeEnvio(tSolicitudDeFinalizacion,
			errorMessage->tamanio + sizeof(int));
	empaquetarCadena(paquete, errorMessage);
	return paquete;
}

bool* deserializarSolicitudDeFinalizacion(package* paquete) {
	string* respuesta = malloc(sizeof(string));
	desempaquetarCadena(paquete, respuesta);
	return respuesta;
}

package* serializarSolicitudEncardoGlobal(void* data) {
	pedidoReduccionGlobal* respuesta = (pedidoReduccionGlobal*) data;
	int tamanioTotal = getStringSize(&respuesta->pathArchivo);
	package* paquete = crearPaqueteDeEnvio(tSolicitudEncargadoGlobal,
			tamanioTotal);
	//Empaqueto los componentes
	empaquetarCadena(paquete, &respuesta->pathArchivo);
	return paquete;
}

pedidoReduccionGlobal* deserializarSolicitudEncardoGlobal(package* paquete) {
	pedidoReduccionGlobal * respuesta = malloc(sizeof(pedidoReduccionGlobal));
	desempaquetarCadena(paquete, &respuesta->pathArchivo);
	return respuesta;
}

package* serializarRespuestaEntradaNodo(void* data) {
	respuestaEntradaNodo * respuesta = (respuestaEntradaNodo*) data;
	int tamanioTotal = sizeof(int);
	package* paquete = crearPaqueteDeEnvio(tRespuestaEntradaNodo, tamanioTotal);
	//Empaqueto los componentes
	empaquetarEntero(paquete, &respuesta->respuesta);
	return paquete;
}

respuestaEntradaNodo * deserializarRespuestaEntradaNodo(package* paquete) {
	respuestaEntradaNodo * respuesta = malloc(sizeof(respuestaEntradaNodo));
	desempaquetarEntero(paquete, &respuesta->respuesta);
	return respuesta;
}

package* serializarRespuestaMetadataGuardarArchivo(void* data) {
	respuestaMetadataGuardarArchivo * respuesta =
			(respuestaMetadataGuardarArchivo*) data;
	int tamanioTotal = sizeof(int);
	package* paquete = crearPaqueteDeEnvio(tRespuestaMetadataGuardarArchivo,
			tamanioTotal);
	//Empaqueto los componentes
	empaquetarEntero(paquete, &respuesta->respuesta);
	return paquete;
}

respuestaMetadataGuardarArchivo * deserializarRespuestaMetadataGuardarArchivo(
		package* paquete) {
	respuestaMetadataGuardarArchivo * respuesta = malloc(
			sizeof(respuestaMetadataGuardarArchivo));
	desempaquetarEntero(paquete, &respuesta->respuesta);
	return respuesta;
}

package* serializarSolicitudDeAlmacenamientoFinal(string* request) {
	package* paquete = crearPaqueteDeEnvio(tSolicitudDeAlmacenamientoFinal,
			sizeof(int) + request->tamanio);
	empaquetarCadena(paquete, request);
	return paquete;
}

string* deserializarSolicitudDeAlmacenamientoFinal(package* paquete) {
	string* respuesta = malloc(sizeof(string));
	desempaquetarCadena(paquete, respuesta);
	return respuesta;
}

package* serializarSolicitudBloqueGuardarArchivo(void* data) {
	string * respuesta = (string *) data;
		int tamanioTotal = getStringSize(respuesta);
		package* paquete = crearPaqueteDeEnvio(tSolicitudBloqueGuardarArchivo,	tamanioTotal);

		//Empaqueto los componentes
		empaquetarCadena(paquete, respuesta);
		return paquete;
}

solicitudBloqueGuardarArchivo * deserializarSolicitudBloqueGuardarArchivo(
		package* paquete) {
	solicitudBloqueGuardarArchivo * respuesta = malloc(
			sizeof(solicitudBloqueGuardarArchivo));
	desempaquetarCadena(paquete, &respuesta->bloque);
	return respuesta;
}

package* serializarRespuestaBloqueGuardarArchivo(void* data) {
	respuestaBloqueGuardarArchivo * respuesta =
			(respuestaBloqueGuardarArchivo*) data;
	int tamanioTotal = sizeof(int);
	package* paquete = crearPaqueteDeEnvio(tRespuestaBloqueGuardarArchivo,
			tamanioTotal);
	//Empaqueto los componentes
	empaquetarEntero(paquete, &respuesta->respuesta);
	return paquete;
}

respuestaBloqueGuardarArchivo * deserializarRespuestaBloqueGuardarArchivo(package* paquete) {
	respuestaBloqueGuardarArchivo * respuesta = malloc(
			sizeof(respuestaBloqueGuardarArchivo));
	desempaquetarEntero(paquete, &respuesta->respuesta);
	return respuesta;
}


package* serializarScript(void* data) {
	string * respuesta = (string *) data;
	int tamanioTotal = getStringSize(respuesta);
	package* paquete = crearPaqueteDeEnvio(tScript,	tamanioTotal);
	//Empaqueto los componentes
	empaquetarCadena(paquete, respuesta);
	return paquete;
}
string * deserializarScript(package* paquete) {
	string * respuesta = malloc(sizeof(string));
	desempaquetarCadena(paquete, respuesta);
	return respuesta;
}

package* serializarPing(void* data) {
	ping * pin = (ping *) data;
	int tamanioTotal = sizeof(ping);
	package* paquete = crearPaqueteDeEnvio(tPing,tamanioTotal);
	//Empaqueto los componentes
	empaquetarEntero(paquete, &pin->ping);
	return paquete;
}
ping * deserializarPing(package* paquete) {
	ping * pin = malloc(sizeof(ping));
	desempaquetarEntero(paquete,&pin->ping);
	return pin;
}


void * deserializar(int tipoMensaje, void * mensaje, int tamanio) {
	void * resultado;
	package* paquete = malloc(sizeof(package));
	paquete->buffer = mensaje;
	paquete->tamanio = tamanio;
	paquete->desplazamiento = 0;

	switch (tipoMensaje) {
	case tTransformacion: {
		resultado = deserializarTransformacion(paquete);
		break;
	}
	case tReduccionLocal: {
		resultado = deserializarReduccion(paquete);
		break;
	}

	case tSolicitudEncargadoGlobal: {
		resultado = deserializarSolicitudEncardoGlobal(paquete);
		break;
	}
	case tReduccionGlobal: {
		resultado = deserializarReduccionGlobal(paquete);
		break;
	}
	case tEntradaNodo: {
		resultado = deserializarEntradaNodo(paquete);
		break;
	}
	case tSolicitudTransformacion: {
		resultado = deserializarSolicitudTransformacion(paquete);
		break;
	}
	case tSolicitudBloquesArchivo: {
		resultado = deserializarSolicitudBloquesArchivo(paquete);
		break;
	}
	case tRespuestaTransformacion: {
		resultado = deserializarRespuestaTransformacion(paquete);
		break;
	}
	case tRespuestaBloquesArchivo: {
		resultado = deserializarRespuestaBloqueArchivo(paquete);
		break;
	}
	case tSolicitudReduccionGlobal: {
		resultado = deserializarSolicitudReduccionGlobal(paquete);
		break;
	}
	case tRespuestaAlmacenadoFinal: {
		resultado = deserializarRespuestaAlmacenadoFinal(paquete);
		break;
	}
	case tConfirmacionFinalizacion: {
		resultado = deserializarConfirmacionFinalizacion(paquete);
		break;
	}
	case tSolicitudSetBloque: {
		resultado = deserializarSolicitudSetBloque(paquete);
		break;
	}
	case tSolicitudMetadataGuardarArchivo: {
		resultado = deserializarSolicitudMetadataGuardarArchivo(paquete);
		break;
	}
	case tSolicitudGetBloque: {
		resultado = deserializarSolicitudGetBloque(paquete);
		break;
	}
	case tRespuestaSetBloque: {
		resultado = deserializarRespuestaSetBloque(paquete);
		break;
	}
	case tRespuestaGetBloque: {
		resultado = deserializarRespuestaGetBloque(paquete);
		break;
	}
	case tRespuestaReduccionLocal: {
		resultado = deserializarRespuestaReduccionLocal(paquete);
		break;
	}
	case tSolicitudDeFinalizacion: {
		resultado = deserializarSolicitudDeFinalizacion(paquete);
		break;
	}
	case tRespuestaEntradaNodo: {
		resultado = deserializarRespuestaEntradaNodo(paquete);
		break;
	}
	case tRespuestaMetadataGuardarArchivo: {
		resultado = deserializarRespuestaMetadataGuardarArchivo(paquete);
		break;
	}
	case tSolicitudDeAlmacenamientoFinal: {
		resultado = deserializarSolicitudDeAlmacenamientoFinal(paquete);
		break;
	}
	case tSolicitudBloqueGuardarArchivo: {
		resultado = deserializarSolicitudBloqueGuardarArchivo(paquete);
		break;
	}
	case tRespuestaBloqueGuardarArchivo: {
		resultado = deserializarRespuestaBloqueGuardarArchivo(paquete);
		break;
	}
	case tScript: {
		resultado = deserializarScript(paquete);
			break;
		}
	case tPing: {
			resultado = deserializarPing(paquete);
				break;
			}
	case 0: {
		printf("Desconexion\n");
		break;
	}
	default: {
		perror("Se recibio un mensaje que no esta en el protocolo");
		abort();
		break;
	}
	}

	free(paquete);
	return resultado;
} // Recordar castear

void* serializar(int tipoMensaje, void* elemento) {
	package* paquete;
	switch (tipoMensaje) {
	case tTransformacion: {
		paquete = serializarTransformacion(elemento);
		break;
	}
	case tReduccionLocal: {
		paquete = serializarReduccion(elemento);
		break;
	}

	case tSolicitudEncargadoGlobal: {
		paquete = serializarSolicitudEncardoGlobal(elemento);
		break;
	}
	case tReduccionGlobal: {
		paquete = serializarReduccionGlobal(elemento);
		break;
	}
	case tEntradaNodo: {
		paquete = serializarEntradaNodo(elemento);
		break;
	}
	case tSolicitudTransformacion: {
		paquete = serializarSolicitudTransformacion(elemento);
		break;
	}
	case tSolicitudBloquesArchivo: {
		paquete = serializarSolicitudBloquesArchivo(elemento);
		break;
	}
	case tRespuestaTransformacion: {
		paquete = serializarRespuestaTransformacion(elemento);
		break;
	}
	case tRespuestaBloquesArchivo: {
		paquete = serializarRespuestaBloqueArchivo(elemento);
		break;
	}
	case tSolicitudReduccionGlobal: {
		paquete = serializarSolicitudReduccionGlobal(elemento);
		break;
	}
	case tRespuestaAlmacenadoFinal: {
		paquete = serializarRespuestaAlmacenadoFinal(elemento);
		break;
	}
	case tConfirmacionFinalizacion: {
		paquete = serializarConfirmacionFinalizacion(elemento);
		break;
	}
	case tSolicitudSetBloque: {
		paquete = serializarSolicitudSetBloque(elemento);
		break;
	}
	case tSolicitudMetadataGuardarArchivo: {
		paquete = serializarSolicitudMetadataGuardarArchivo(elemento);
		break;
	}
	case tSolicitudGetBloque: {
		paquete = serializarSolicitudGetBloque(elemento);
		break;
	}
	case tRespuestaSetBloque: {
		paquete = serializarRespuestaSetBloque(elemento);
		break;
	}
	case tRespuestaGetBloque: {
		paquete = serializarRespuestaGetBloque(elemento);
		break;
	}
	case tRespuestaReduccionLocal: {
		paquete = serializarRespuestaReduccionLocal(elemento);
		break;
	}
	case tSolicitudDeFinalizacion: {
		paquete = serializarSolicitudDeFinalizacion(elemento);
		break;
	}
	case tRespuestaEntradaNodo: {
		paquete = serializarRespuestaEntradaNodo(elemento);
		break;
	}
	case tRespuestaMetadataGuardarArchivo: {
		paquete = serializarRespuestaMetadataGuardarArchivo(elemento);
		break;
	}
	case tSolicitudDeAlmacenamientoFinal: {
		paquete = serializarSolicitudDeAlmacenamientoFinal(elemento);
		break;
	}
	case tSolicitudBloqueGuardarArchivo: {
		paquete = serializarSolicitudBloqueGuardarArchivo(elemento);
		break;
	}
	case tRespuestaBloqueGuardarArchivo: {
		paquete = serializarRespuestaBloqueGuardarArchivo(elemento);
		break;
	}
	case tScript: {
			paquete = serializarScript(elemento);
			break;
		}
	case tPing: {
		paquete = serializarPing(elemento);
		break;
				}
	case 0: {
		//printf("Desconexion\n");
		break;
	}
	default: {
		//perror("Se recibio un mensaje que no esta en el protocolo");
		abort();
		break;
	}
	}
	return paquete;
}

void enviarPaquete(int fdCliente, int tipoMensaje, void * mensaje,
		int tamanioMensaje) {
	package *paquete = (package*) serializar(tipoMensaje, mensaje);
	if (paquete->desplazamiento != paquete->tamanio) {
		printf(
				"Error en serializacion. Tipo de mensaje:%d Desplazamiento:%d Tamanio calculado:%d\n",
				tipoMensaje, paquete->desplazamiento, paquete->tamanio);
	}

	enviarPorSocket(fdCliente, paquete->buffer, paquete->tamanio);
	free(paquete->buffer);
	free(paquete);
}

void * recibirPaquete(int fdCliente, int * tipoMensaje, int * tamanioMensaje) {
	// Recibe el tipoMensaje
	int recibido = recibirPorSocket(fdCliente, tipoMensaje, sizeof(int));

	if (*tipoMensaje < 1 || *tipoMensaje > tFinDeProtocolo || recibido <= 0) {
		return NULL;
	}

	// Recibe el tamanioMensaje
	recibirPorSocket(fdCliente, tamanioMensaje, sizeof(int));

	void * mensaje = malloc(*tamanioMensaje);
	recibirPorSocket(fdCliente, mensaje, *tamanioMensaje);
	void * resultado = deserializar(*tipoMensaje, mensaje, *tamanioMensaje);
	free(mensaje);
	return resultado;
} // Recordar castear

int getStringSize(string *cadena) {
	return (sizeof(char) * cadena->tamanio) + sizeof(int);
}


