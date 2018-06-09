#!/bin/bash
gcc -I/home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Consola.d" -MT"src/Consola.o" -o "src/Consola.o" "../src/Consola.c"
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
git checkout Check_2
