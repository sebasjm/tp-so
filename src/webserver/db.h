#ifndef __webserver_db_h
#define __webserver_db_h

typedef struct __cursor_list {
    char* value;
    char* key;
    struct __cursor_list* next;
} cursor_list;

int db_error_set_value(char* key_value, char* data_value);
char* db_error_get_value(char* key_value);
cursor_list* db_error_get_all();

int db_done_set_value(char* key_value, char* data_value);
char* db_done_get_value(char* key_value);
cursor_list* db_done_get_all();

void free_cursor_list(cursor_list* list);

HANDLE get_handle();

#endif
