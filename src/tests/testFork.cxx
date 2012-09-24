#include <iostream>
#include <sys/ipc.h>

using namespace std;

int main() {
    pid_t threadID = fork(); // en este punto el programa se bifurca
    
    if (threadID) { // la unica diferencia entre los dos es este valor
        cout << "padre" << endl;
    } else {
        cout << "hijo" << endl;
    }
    cout << "ok" << endl;
    return 0;
}
