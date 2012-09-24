#include <xml/configuration.h>

#include <util/config.h>
#include <iostream>

using namespace std;

int main(int argc,char** argv) {
    cout << "arch " << argv[1] << " prop " << argv[2] << " value " << read_config(argv[1],argv[2]) << endl;
    return 0;
}
