#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "template.h"

#include <util/log.h>
__init_log(str_template)

using namespace std;

char* dynamic_replace_var(char* linea, char* var, const char* value) {
    char* result = replace_var(linea,var,value);
    free(linea);
    return result;
}

char* replace_var(char* linea, char* var, const char* value) {
    char* varname = (char*)malloc( strlen(var) + 3 + 1);
    varname[0] = '\0';
    strcat(varname,"${");
    strcat(varname,var);
    strcat(varname,"}");
    varname[strlen(var) + 3] = '\0';

    char* result = str_replace(linea, varname, value);
    if (!result) {
        error << " el str_replace no devolvio nada " << endl;
    }
    free(varname);
    return result;
}

//todo: verificar que cuando haces el reemplazo no se pase del maximo
// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, const char *value) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep
    int len_with; // length of with
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    if (!orig)
        return NULL;
    if (!rep || !(len_rep = strlen(rep)))
        return NULL;
    if (!(ins = strstr(orig, rep))) {
        int length = strlen(orig);
        char* result = (char*)malloc(length+1);
        strcpy(result,orig);
        result[length] = '\0';
        return result;
    }
    if (!value)
        value = "";
    len_with = strlen(value);

    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + 1;
    }

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    tmp = result = (char*)malloc( strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, value) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
        count--;
    }
    strcpy(tmp, orig);
    return result;
}



