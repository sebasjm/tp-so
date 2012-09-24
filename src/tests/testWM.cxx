#include <stdio.h>
#include <node/dc/webManager.h>
#include <messages/dc-messages.h>
#include <util/log_ansi.h>
#define modulo "testWM"
__init_logname(modulo)

void tm_getInfo(DCState* state){

}
void nm_getInfo(DCState* state){

}
int main(){
/*
    initWM();
*/
    startWM();
    stopWM();
}
