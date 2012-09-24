#ifndef __str_template_h
#define __str_template_h

char* str_replace(char *orig, char *rep, const char *value);
char* dynamic_replace_var(char* linea, char* var, const char* value);
char* replace_var(char* linea, char* var, const char* value);

#endif
