#!/bin/bash

echo "compilo Coordinador"
gcc -Wall  /home/utnso/workspace/tp-2018-1c-Fail-system/Coordinador/src/Coordinador.c -o Coordinador   -lcommons -lpthread -lhelper -lsockets

echo "compilo Planificador"
gcc -Wall /home/utnso/workspace/tp-2018-1c-Fail-system/Planificador/src/Planificador.c /home/utnso/workspace/tp-2018-1c-Fail-system/Planificador/src/Consola.c -o Planificador   -lcommons -lpthread -lreadline -lhelper -lsockets
echo "compilo ESI"
gcc -Wall  /home/utnso/workspace/tp-2018-1c-Fail-system/ESI/src/ESI.c -o ESI   -lcommons -lpthread -lparsi -lhelper -lsockets
echo "compilo Instancia"
gcc -Wall   /home/utnso/workspace/tp-2018-1c-Fail-system/Instancia/src/Instancia.c  -o Instancia   -lcommons -lpthread  -lhelper -lsockets
echo "Finalizo compilación"
