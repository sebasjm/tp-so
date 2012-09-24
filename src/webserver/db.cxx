#include <string.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

#include <util/log.h>
__init_log(webserver_db)

#include <sys/types.h>

#include <so_agnostic.h>
#ifdef __CHANCHADAS_MARCA_WINDOWS
#include <libs/berkeley/db_win.h>
#else
#include <libs/berkeley/db.h>
#endif

#include "db.h"

DB* db = NULL;
DBC *dbcp = NULL;

DB* db_get_instance(char *db_name) {
    if (db) return db;
    int ret;
    if ((ret = db_create(&db, NULL, 0)) != 0) {
        error << "db_create: " << db_strerror(ret) << endl;
        return NULL;
    }
    if ((ret = db->open(db, NULL, db_name , NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
        db->err(db, ret, "%s", db_name );
        error << "db_open: " << db_strerror(ret) << endl;
        return NULL;
    }
    if ((ret = db->cursor(db, NULL, &dbcp, 0)) != 0) {
        db->err(db, ret, "DB->cursor");
        return NULL;
    }

    return db;
}

void db_close() {
    dbcp->c_close(dbcp);
    db->close( db , 0);
    db = NULL;
    dbcp = NULL;
}

int db_set_value(char* db_name, char* key_value, char* data_value) {
    DBT key, data;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.data = key_value;
    key.size = strlen(key_value);
    data.data = data_value;
    data.size = strlen(data_value);
    
    db_get_instance(db_name);

    int result = dbcp->c_put(dbcp, &key, &data, DB_KEYLAST );
    
    db_close();
    return result;
}

char* db_get_value(char* db_name, char* key_value) {
    DBT key, data;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.data = key_value;
    key.size = strlen(key_value);

    db_get_instance(db_name);
    dbcp->c_get(dbcp, &key, &data,  DB_SET);

    char* result = NULL;
    if (data.size) {
        result = (char*) malloc(data.size+1);
        memcpy(result,data.data,data.size);
        result[data.size] = '\0';
    }

    db_close();
    return result;
}

HANDLE h = NULL;

HANDLE get_handle(){
	if (h == NULL) h = HeapCreate(0,0,0);
	return h;
}

cursor_list* db_get_all(char* db_name) {
    cursor_list* list = NULL;
    DBT key, data;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    int ret;

    db_get_instance(db_name);

    cursor_list* last = NULL;
    while ((ret = dbcp->c_get(dbcp, &key, &data, DB_NEXT)) == 0) {
        char* result = (char*) HeapAlloc(get_handle(),0,data.size+1);
        memcpy(result, data.data, data.size);
        result[data.size] = '\0';

        char* result_key = (char*) HeapAlloc(get_handle(),0,key.size+1);
        memcpy(result_key, key.data, key.size);
        result_key[key.size] = '\0';

        cursor_list* value = new cursor_list;
        value->next = NULL;
        value->value = result;
        value->key = result_key;

        if (last == NULL) {
            list = value;
            last = list;
        } else {
            last->next = value;
            last = last->next;
        }

    }
    db_close();
    
    return list;
}

void free_cursor_list(cursor_list* list) {
    cursor_list* ant = NULL;
    while(list) {
        HeapFree(get_handle(),0,list->value);
        ant = list;
        list = list->next;
        free(ant);
    }
}

/////

int db_error_set_value(char* key_value, char* data_value) {
    return db_set_value("webserver_errorQueue.db",key_value,data_value);
}
char* db_error_get_value(char* key_value){
    return db_get_value("webserver_errorQueue.db",key_value);
}
cursor_list* db_error_get_all(){
    return db_get_all  ("webserver_errorQueue.db");
}

int db_done_set_value(char* key_value, char* data_value){
    return db_set_value("webserver_doneQueue.db",key_value,data_value);
}
char* db_done_get_value(char* key_value){
    return db_get_value("webserver_doneQueue.db",key_value);
}
cursor_list* db_done_get_all(){
    return db_get_all  ("webserver_doneQueue.db");
}



