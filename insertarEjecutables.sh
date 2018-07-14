#!/bin/bash
echo Copio ejecutables
sudo cp /home/utnso/workspace/tp-2018-1c-Fail-system/Coordi.sh /home/utnso/
sudo cp /home/utnso/workspace/tp-2018-1c-Fail-system/Plani.sh /home/utnso/
cd
ln -s workspace/tp-2018-1c-Fail-system/ESI/Debug/./ESI
ln -s workspace/tp-2018-1c-Fail-system/Instancia/Debug/./Instancia 
ln -s workspace/tp-2018-1c-Fail-system/Instancia/Instancia.cfg
ln -s workspace/tp-2018-1c-Fail-system/ESI/esi.cfg 
echo Finalizo copia
