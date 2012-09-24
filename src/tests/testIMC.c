/*
 * testIMC.c
 *
 *  Created on: 31/10/2010
 *      Author: pmarchesi
 */

#include <stdio.h>
#include <node/dc/imc.h>
#include <unistd.h>

int testPass = 0;
int valueTest2 = 8;

#include <util/log_ansi.h>
#define modulo "testIMC"
__init_logname(modulo)

void test1(void* null){
	debug(modulo,"Test -> newNodo\n");
	sleep(1);
	testPass = testPass +1;
}
void test1_2(void* null){
	debug(modulo,"Test -> newNodo2\n");
	testPass = testPass +1;
}

void test2(int* value){
	debug(modulo,"Test -> newJobs	\n");
	if (*value == valueTest2){
		testPass = testPass +1;
	}else{
		debug(modulo, "Valor recibido %d se esperaba %d\n", *value, valueTest2);
	}
}

void tm_getInfo(DCState* state){

}
void nm_getInfo(DCState* state) {

}
void wm_getInfo(DCState* state) {
    
}

int main() {
	 initIMC();
	 startIMC();
	 debug(modulo, "function to suscribe: %d  type %d\n", &test1, NEW_NODE);
	 debug(modulo, "function to suscribe: %d  type %d\n", &test1_2, NEW_NODE);
	 debug(modulo, "function to suscribe: %d  type %d\n", &test2, JOBS_ENDED);
	 imc_suscribe(NEW_NODE,(Function*)&test1);
	 imc_suscribe(JOBS_ENDED,(Function*)&test2);
	 imc_suscribe(NEW_NODE,(Function*)&test1_2);
	 imc_notify(NEW_NODE, NULL);
	 imc_notify(JOBS_ENDED,&valueTest2);
	 imc_notify(NEW_NODE, NULL);
	 imc_notify(NEW_NODE, NULL);
	 imc_notify(JOBS_ENDED,&valueTest2);
	 imc_notify(JOBS_ENDED,&valueTest2);
	 sleep(4);
	 debug(modulo, "Test Pasados: %d\n", testPass);
	 if (testPass == 9)
		 printf("ok\n");
	 stopIMC();
	 return 0;
}
