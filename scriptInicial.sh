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

