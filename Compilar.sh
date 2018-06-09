#!/bin/bash

echo "compilo Coordinador"
gcc -Wall  /home/utnso/workspace/tp-2018-1c-Fail-system/Coodinador/src/Coordinador.c  /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/sockets.c /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/helper.c -o Coordinador   -lcommons -lpthread

echo "compilo Planificador"
gcc -Wall /home/utnso/workspace/tp-2018-1c-Fail-system/Planificador/src/ Planificador.c  /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/sockets.c /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/helper.c /home/utnso/workspace/tp-2018-1c-Fail-system/Planificador/src/Consola.c  -o Planificador   -lcommons -lpthread -lreadline
echo "compilo ESI"
gcc -Wall  /home/utnso/workspace/tp-2018-1c-Fail-system/ESI/src/ESI.c  /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/sockets.c /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/helper.c -o ESI   -lcommons -lpthread -lparsi
echo "compilo Instancia"
gcc -Wall   /home/utnso/workspace/tp-2018-1c-Fail-system/Instancia/src/Instancia.c  /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/sockets.c /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/helper.c -o Instancia   -lcommons -lpthread 
echo "Finalizo compilación"
