#!/bin/bash

echo -e "Testing nodos\n----------------"
for nodo in $(ls config/nodos/$1/*.xml)
do 
	sleep 1
	echo -e "\n *********** Iniciando nodo $nodo"
	bash -c "./build/node/daemon/daemon.bin $nodo &"
done

sleep 1

echo -e "\nResultados\n----------------"
for nodo in $(ls config/nodos/$1/*.xml)
do 
	nodoName=$(echo $nodo | sed "s/.xml//g" | sed "s/config\/nodos\///g")
	echo ""
	echo -e "$nodoName display\nexit\n" | ./build/node/interface/interface.bin
done

echo -ne "\nMatando nodos: \n\n"
for nodo in $(ls config/nodos/$1/*.stop)
do 
	sleep 1
	echo -n $nodo " . "
	nodoName=$(echo $nodo | sed "s/.stop//g" | sed "s/config\/nodos\///g")
	echo -e "$nodoName stop\nexit\n" | ./build/node/interface/interface.bin
	
	sleep 2
	echo -e "\nResultados\n----------------"
	for nodo in $(ls config/nodos/$1/*.xml)
	do 
		nodoName=$(echo $nodo | sed "s/.xml//g" | sed "s/config\/nodos\///g")
		echo ""
		echo -e "$nodoName display\nexit\n" | ./build/node/interface/interface.bin
	done
done

#	sleep 15
#	echo -e "\nResultados\n----------------"
#	for nodo in $(ls config/nodos/$1/*.xml)
#	do 
#		nodoName=$(echo $nodo | sed "s/.xml//g" | sed "s/config\/nodos\///g")
#		echo ""
#		echo -e "$nodoName display\nexit\n" | ./build/node/interface/interface.bin
#	done

#killall daemon.bin
