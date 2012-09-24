#include <string.h>
#include <strings.h>
#include <md5/md5.h>

int main (int argc, char *argv[])
{
	char buf[255] = "Pepita vola";
	unsigned char res[32];
	unsigned char resStr[33];
	unsigned char encrip[33] ="e5ea77e258aad38f0a3354d87b1a9067";
	int i;
	memset ((void*)res, (int)'\0', (size_t)sizeof(res));
	md5_buffer (buf, strlen(buf), res);
	printf ("Hash de %s:", buf);
	for (i=0; i<16; i++){
		printf ("%02x", res[i]);
		sprintf ((char*)&resStr[i*2],"%02x",res[i]);
	}
	printf("\n");
	if (strncmp((char*)resStr,(char*)encrip,32) == 0)
		printf("Test ok\n");
	else
		printf("Test fail. Expected encriptation string %s\n", encrip);
	return 0;
}

