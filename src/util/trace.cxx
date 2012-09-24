#include "trace.h"

#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>

void printBackTrace() __attribute__ ((no_instrument_function));
void printBackTrace() {
    void* callstack[128];
    int i, frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
//    for (i = 0; i < frames; ++i) {
        fprintf(stderr,"%s\n", strs[2]);
//    }
    free(strs);
}

extern "C" {
    
    void __cyg_profile_func_enter (void *this_fn,void *call_site) {
        printBackTrace();
    }

    void __cyg_profile_func_exit  (void *this_fn,void *call_site) {

    }

}

