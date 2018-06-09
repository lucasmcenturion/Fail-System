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

echo "compilo bibliotecas"
gcc -I/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/helper.d" -MT"src/helper.o" -o "src/helper.o" "/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/helper.c"
gcc -I/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/sockets.d" -MT"src/sockets.o" -o "src/sockets.o" "/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/sockets.c"

echo "compilo Coordinador"
gcc -I/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Coordinador.d" -MT"src/Coordinador.o" -o "src/Coordinador.o" "../src/Coordinador.c"
gcc  -o "Coordinador"  ./src/Coordinador.o ./src/helper.o ./src/sockets.o   -lcommons -lpthread


echo "compilo Planificador"
gcc -I/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Consola.d" -MT"src/Consola.o" -o "src/Consola.o" "../src/Consola.c"
gcc -I/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Planificador.d" -MT"src/Planificador.o" -o "src/Planificador.o" "../src/Planificador.c"gcc  -o "Planificador"  ./src/Consola.o ./src/Planificador.o ./src/helper.o ./src/sockets.o   -lcommons -lreadline -lpthread
gcc  -o "Planificador"  ./src/Consola.o ./src/Planificador.o ./src/helper.o ./src/sockets.o   -lcommons -lreadline -lpthread

echo "compilo ESI"
gcc -I/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/ESI.d" -MT"src/ESI.o" -o "src/ESI.o" "../src/ESI.c"
gcc  -o "ESI"  ./src/ESI.o ./src/helper.o ./src/sockets.o   -lcommons -lparsi -lpthread

echo "compilo Instancia"
gcc -I/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Instancia.d" -MT"src/Instancia.o" -o "src/Instancia.o" "../src/Instancia.c"
gcc  -o "Instancia"  ./src/Instancia.o ./src/helper.o ./src/sockets.o   -lcommons -lpthread

echo "Finalizo compilación"
