#!/bin/bash

echo -e "\nSystem Status\n----------------"
for nodo in $(ls config/nodos/$1/*.xml)
do 
	nodoName=$(echo $nodo | sed "s/.xml//g" | sed "s/config\/nodos\///g")
	echo "$nodoName"
	echo -e "$nodoName display\nexit\n" | ./build/node/interface/interface.bin
done

