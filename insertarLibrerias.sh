#!/bin/bash

echo Instalando helper.h
cd /home/utnso/workspace/tp-2018-1c-Fail-system/Bibliotecas/
echo compilo  helper.h
gcc -static -c helper.c  -o helper.o
echo creo biblioteca helper.h
ld -o libhelper.so helper.o -shared
echo Copio lib en include
sudo cp -u helper.h /usr/include
sudo cp -u libhelper.so /usr/lib
echo Instalando sockets.h
gcc -static -c sockets.c  -o sockets.o -lhelper
ld -o libsockets.so sockets.o -shared
sudo cp -u sockets.h /usr/include
sudo cp -u libsockets.so /usr/lib
echo Finalizo instalación sockets.h y helper.h