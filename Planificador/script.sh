#!/bin/bash

mkdir workspace
cd workspace
git clone https://github.com/sisoputnfrba/so-commons-library
git clone https://github.com/sisoputnfrba/parsi
git clone https://github.com/sisoputnfrba/Pruebas-ESI
git clone https://github.com/sisoputnfrba/tp-2018-1c-Fail-system

cd so-commons-library
sudo make install
cd ..
cd parsi
sudo make install 
cd 	..
cd tp-2018-1c-Fail-system
git checkout Check_3

gcc -I/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Consola.d" -MT"src/Consola.o" -o "src/Consola.o" "../src/Consola.c"
gcc -I/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Planificador.d" -MT"src/Planificador.o" -o "src/Planificador.o" "../src/Planificador.c"
gcc -I/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/helper.d" -MT"src/helper.o" -o "src/helper.o" "/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/helper.c"
gcc -I/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/sockets.d" -MT"src/sockets.o" -o "src/sockets.o" "/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/sockets.c"
gcc  -o "Planificador"  ./src/Consola.o ./src/Planificador.o ./src/helper.o ./src/sockets.o   -lcommons -lreadline -lpthread
