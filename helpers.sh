El codigo de abajo agrega refresh_time="2" a todos los xmls
La primera saca todo menos el close tag
El segundo agrega la propiedad al archivo nuevo
La tercera cierra el tag
La cuarta cambia el nombre del archivo


for file in {1..8}; do head -n 11 ${file}.xml > ${file}.xml2; done
for file in {1..8}; do echo refresh_time=\"2\" >> ${file}.xml2; done
for file in {1..8}; do echo /\> >> ${file}.xml2; done
for file in {1..8}; do mv ${file}.xml2 ${file}.xml; done
