#include <commons/collections/list.h>

//////////////////////////////////////////
//              PROTOCOLO        		//
//////////////////////////////////////////
typedef enum t_protocolo {
	tTransformacion = 1,
	tEntradaNodo,
	tSolicitudTransformacion,
	tSolicitudBloquesArchivo,
	tRespuestaTransformacion,
	tRespuestaReduccionLocal,
	tSolicitudReduccionGlobal,
	tRespuestaAlmacenadoFinal,
	tConfirmacionFinalizacion,
	tRespuestaBloquesArchivo,
	tSolicitudSetBloque,
	tSolicitudMetadataGuardarArchivo,
	tReduccionLocal,
	tSolicitudGetBloque,
	tRespuestaSetBloque,
	tRespuestaGetBloque,
	tSolicitudEncargadoGlobal,
	tReduccionGlobal,
	tSolicitudDeFinalizacion,
	tRespuestaEntradaNodo,
	tRespuestaMetadataGuardarArchivo,
	tSolicitudDeAlmacenamientoFinal,
	tSolicitudBloqueGuardarArchivo,
	tRespuestaBloqueGuardarArchivo,
	tScript,
	tPing,
	tFinDeProtocolo //NO SACAR Y DEJAR A LO ULTIMO!!!
} t_protocolo;

typedef enum tipoDeNodos {
	tnodoRespuestaTransformacion = 1,
	tnodoRespuestaBloquesArchivo,
	tnodoSolicitudReduccionGlobal,
	tnodoRespuestaReduccionLocal
} tipoDeNodos;

typedef enum step {
	TRANSFORMACION, REDUCCION_LOCAL, REDUCCION_GLOBAL, ALMACENAMIENTO
} step;
typedef enum proceso {
	FILESYSTEM, YAMA, DATANODE, WORKER, MASTER
} proceso;
typedef enum {
	TEXTO = 0, BINARIO
} tipoTexto;

typedef struct package {
	void* buffer;
	int desplazamiento;
	int tamanio;
}__attribute__((packed)) package;

typedef struct string {
	int tamanio;
	char* cadena;
}__attribute__((packed)) string;

typedef struct ping {
	int ping;
}__attribute__((packed)) ping;


////////////////////////////////////////////
///     Master <-> WORKER               ///
////////////////////////////////////////////
typedef struct transformacion {
	int numeroDeBloque;
	int cantidadDeBytes;
	string pathTemporal;
}__attribute__((packed)) transformacion;

typedef struct reduccionLocal {
	string pathTemporal;
	t_list* archivosTemporales;
}__attribute__((packed)) reduccionLocal;

typedef struct pedidoReduccionGlobal {
	string pathArchivo;
}__attribute__((packed)) pedidoReduccionGlobal;


////////////////////////////////////////////
///     DATANODE <-> FS <-> WORKER       ///
////////////////////////////////////////////
typedef struct t_entradaNodo {
	string idNodo;
	string ipNodo;
	int puertoWorker;
	int tamanioDataBin;
}__attribute__((packed)) t_entradaNodo;

typedef struct respuestaEntradaNodo {
	int respuesta; //0:ok -1:fallido
}__attribute__((packed)) respuestaEntradaNodo;

typedef struct solicitudSetBloque {
	int bloque;
	string datos;
}__attribute__((packed)) solicitudSetBloque;

typedef struct solicitudGetBloque {
	int bloque;
}__attribute__((packed)) solicitudGetBloque;

typedef struct respuestaGetBloque{
	int respuesta; //0:ok -1:fallido
	string datos;
}__attribute__((packed)) respuestaGetBloque;

typedef struct respuestaSetBloque {
	int respuesta; //0:ok -1:fallido
}__attribute__((packed)) respuestaSetBloque;

typedef struct solicitudMetadataGuardarArchivo{
	string ruta;
	string nombreArchivo;
	tipoTexto tipoTexto; // 0:texto 1:binario
	int tamanioArchivo;
	int cantBloques;
}__attribute__((packed)) solicitudMetadataGuardarArchivo;

typedef struct respuestaMetadataGuardarArchivo {
	int respuesta; //0:ok -1:fallido
}__attribute__((packed)) respuestaMetadataGuardarArchivo;

typedef struct solicitudBloqueGuardarArchivo{
	string bloque;
}__attribute__((packed)) solicitudBloqueGuardarArchivo;

typedef struct respuestaBloqueGuardarArchivo {
	int respuesta; //0:ok -1:fallido
}__attribute__((packed)) respuestaBloqueGuardarArchivo;


////////////////////////////////////////////
///                YAMA <-> MASTER       ///
////////////////////////////////////////////
typedef struct t_direccion {
	int puerto;
	string ip;
}__attribute__((packed)) t_direccion;

typedef struct nodo {
	string idNodo;
	int numeroBloque;
	int espacioEnDisco;
	t_direccion direccion;
}__attribute__((packed))  nodo;

typedef struct t_nodo {
	nodo nodoOriginal;
	nodo nodoCopia;
}__attribute__((packed))  t_nodo;

typedef struct t_nodoRespuestaTransformacion {
	nodo nodo;
	string archivoTemporal; // donde guardar la transformacion
}__attribute__((packed))  t_nodoRespuestaTransformacion;

typedef struct t_nodoRespuestaBloquesArchivo {
	int bloqueArchivo;
	t_nodo nodoOriginalCopia; // la dupla de nodos copia y original
}__attribute__((packed))  t_nodoRespuestaBloquesArchivo;

typedef struct solicitudTransformacion {
	string nombreArchivo;
}__attribute__((packed))  solicitudTransformacion;

typedef struct respuestaTransformacion {
	t_list * nodos; //lista de nodos del tipo t_nodoRespuestaTransformacion
}__attribute__((packed))  respuestaTransformacion;

typedef struct respuestaBloquesArchivo {
	int estado; // 0 = ok - 1 Error
	string nombreArchivo;
	t_list * nodos; // lista de nodos del tipo t_nodoRespuestaBloquesArchivo
}__attribute__((packed))  respuestaBloquesArchivo;

typedef struct solicitudBloquesArchivo {
	string nombreArchivo;
}__attribute__((packed))  solicitudBloquesArchivo;

typedef struct t_nodoRespuestaReduccionLocal {
	string archivoTempTransformacion;
}__attribute__((packed))  t_nodoRespuestaReduccionLocal;

typedef struct respuestaReduccionLocal {
	string idNodo;
	t_direccion direccion;
	string archivoTempReduccionLocal;
	t_list * nodos; // lista de nodos del tipo t_nodoRespuestaReduccionLocal
}__attribute__((packed))  respuestaReduccionLocal;

typedef struct t_nodoSolicitudReduccionGlobal {
	string idNodo;
	t_direccion direccion;
	string archivoTempReduccionLocal;
}__attribute__((packed))  t_nodoSolicitudReduccionGlobal;

typedef struct solicitudReduccionGlobal {
	string archivoTempReduccionGlobal;
	string encargado;
	t_list * nodos; // lista de nodos del tipo t_nodoSolicitudReduccionGlobal
} __attribute__((packed)) solicitudReduccionGlobal;


typedef struct respuestaAlmacenadoFinal {
	string idNodo;
	t_direccion direccion;
	string archivoTempReduccionGlobal;
} __attribute__((packed)) respuestaAlmacenadoFinal;

typedef struct confirmacionFinalizacion {
	string idNodo;
	int bloque; // porque sino no se cual de todos los bloques fallo
	step tipoOperacion; // 1=Transformacion - 2=Reduccion Local - 3=Reduccion Global - 4=Almacenamiento
	int estado; // 0=ok - 1=Error
} __attribute__((packed)) confirmacionFinalizacion;

/// MASTER WORKER

typedef struct reduccionGlobal {
	string pathReduccionGlobal;
	t_nodoSolicitudReduccionGlobal *encargado;
	t_list* workers;   // son del tipo t_nodoSolicitudReduccionGlobal
}__attribute__((packed)) reduccionGlobal;

///////////////////////////////////////////
int getStringSize(string *cadena);
void enviarPaquete(int fdCliente, int tipoMensaje, void * mensaje,	int tamanioMensaje);
void * recibirPaquete(int fdCliente, int * tipoMensaje, int * tamanioMensaje);
