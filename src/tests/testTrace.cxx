#include <stdio.h>

void hola();
void decir(char* c);
void decirMundo();

int main() {
    hola();
}

void hola() {
    decir("hola ");
    decirMundo();
}

void decirMundo() {
    decir("Mundo\n");
}

void decir(char* str) {
    printf("%s",str);
}