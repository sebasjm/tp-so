#include <ipc/ipcstream.h>

#include <messages/interface-daemon.h>

#include <iostream>
#include <sys/ipc.h>
#include <iterator>

#include <string>
#include <vector>
#include <algorithm>
       #include <unistd.h>

#include <util/log_ansi.h>
#define modulo "testIpcStream"
__init_logname(modulo)

using namespace std;

int main() {

    if (!fork()) {
        IPCStreamClass<mensaje> ipc("/tmp/sarlanga4");
        mensaje paraMandar = {"un mensaje"};

        ipc << paraMandar;
    } else {
        IPCStreamClass<mensaje> ipc("/tmp/sarlanga4");
        mensaje paraRecibir;

        ipc >> paraRecibir;
        cout << paraRecibir.str << endl;
    }



    return 0;
}
