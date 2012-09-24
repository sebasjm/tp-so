/*
 * characters.c
 * Caracteres validos para una clave.
 *  Created on: 12/11/2010
 *      Author: pmarchesi
 */

char characters[] =  {   48,49,50,51,52,53,54,55,56,57,/* 0-9 */
						65,66,67,68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, /* A-Z */
						97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122 /* a-z */
					};

int numberOfCharactares = (int) sizeof(characters) / sizeof(char);

int positionOfCharacter(char value){
	int i = 0;
	while(i < numberOfCharactares){
		if (value == characters[i])
			return i;
		i++;
	}
	return -1;
}
