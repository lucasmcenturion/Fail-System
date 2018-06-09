#!/bin/bash

mkdir workspace
cd workspace
git clone https://github.com/sisoputnfrba/so-commons-library
git clone https://github.com/sisoputnfrba/parsi
git clone https://github.com/sisoputnfrba/Pruebas-ESI

cd so-commons-library
sudo make install
cd ..
cd parsi
sudo make install 
cd 	..
git clone https://github.com/sisoputnfrba/tp-2018-1c-Fail-system
cd tp-2018-1c-Fail-system
git checkout Check_2
cd Coodinador
cd src
echo "compilo Coordinador"
gcc -Wall  Coordinador.C  /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/sockets.c /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/helper.c -o Coordinador   -lcommons -lpthread
cd ..
cd ..
cd Planificador
cd src
echo "compilo Planificador"
gcc -Wall  Planificador.C  /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/sockets.c /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/helper.c -o Planificador   -lcommons -lpthread -lreadline
cd ..
cd ..
cd ESI
cd src
echo "compilo ESI"
gcc -Wall  ESI.C  /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/sockets.c /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/helper.c -o ESI   -lcommons -lpthread -lparsi
cd ..
cd ..
cd Instancia
cd src
echo "compilo Instancia"
gcc -Wall  Instancia.C  /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/sockets.c /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/helper.c -o Instancia   -lcommons -lpthread 
echo "Finalizo compilación"
cd ..
cd ..
