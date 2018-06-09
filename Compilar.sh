#!/bin/bash

cd /home/utnso/workspace/tp-2018-1c-Fail-system/Coodinador/src
echo "compilo Coordinador"
gcc -Wall  Coordinador.C  /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/sockets.c /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/helper.c -o Coordinador   -lcommons -lpthread
cd ..
cd ..
cd /home/utnso/workspace/tp-2018-1c-Fail-system/Planificador/src
echo "compilo Planificador"
gcc -Wall  Planificador.C  /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/sockets.c /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/helper.c -o Planificador   -lcommons -lpthread -lreadline
cd ..
cd ..
cd /home/utnso/workspace/tp-2018-1c-Fail-system/ESI/src

echo "compilo ESI"
gcc -Wall  ESI.C  /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/sockets.c /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/helper.c -o ESI   -lcommons -lpthread -lparsi
cd ..
cd ..
cd /home/utnso/workspace/tp-2018-1c-Fail-system/Instancia/src
echo "compilo Instancia"
gcc -Wall  Instancia.C  /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/sockets.c /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/helper.c -o Instancia   -lcommons -lpthread 
echo "Finalizo compilación"
cd /home/utnso/workspace/tp-2018-1c-Fail-system/
cd ..
