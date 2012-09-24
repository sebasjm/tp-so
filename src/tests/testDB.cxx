#include <iostream>
#include <stdlib.h>

#include <util/log.h>
#include <webserver/db.h>

__init_log(testDB)
__init_logname(testDB)

int main(int argc,char** argv) {
    if (argc == 1) {
        cursor_list* list, *ptr = db_done_get_all();
        while(ptr) {
            cerr << ptr->key << " -> " << ptr->value << endl;
            ptr = ptr->next;
        }
        free_cursor_list( list );
    }
    if (argc == 2) {
        char* asd = db_done_get_value(argv[1]);
        cerr << (asd ? asd : "nada") << endl;
        free(asd);
    }
    if (argc == 3) {
        db_done_set_value(argv[1],argv[2]);
//        cerr << " argando valor... " << (?"ya existia, o hubo un error":"era nuevo y lo agregue") << endl;
    }
    return 0;
}


