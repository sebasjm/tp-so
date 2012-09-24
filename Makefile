default_target: all

init:
	cd build && rm -rf *

gen generate:
	cd build && cmake ../src/

regen regenerate:
	make init
	make generate

all:
	make -C build $@
clean:
	make -C build $@
test:
	cp -r config build/tests/config
	make -C build $@

refresh:
	make killall
	make rm
	netstat -na | grep TIME || true
	
k killall:	
	make kws
	make kda
	make kdc

kws killws:
	killall webserver.bin || true
kda killdaemon:
	killall daemon.bin || true
kdc killdc:
	killall dc_main.bin || true

rm:
	rm log/*.log || true	
	rm webserver*.db || true
	rm *.pid || true
	make t
	
t ktail killtail:
	killall tail || true
	
ws:
	make killws
	rm -rf /tmp/special_dir
	mkdir /tmp/special_dir/
#	cp src/tests/prueba*.xml /tmp/special_dir/
	cp src/tests/prueba.xml /tmp/special_dir/
	rm webserver*.db || true
	./build/webserver/webserver.bin &

