#ifndef __ipcstream_h
#define __ipcstream_h

#include <fstream>
#include <string>
#include <sys/ipc.h>

#include "point.h"

using namespace std;

template <typename type>
class IPCStreamClass : public PointClass<type> {
public:
    IPCStreamClass(const string nombre);

    PointClass<type>& operator<<(type);
    PointClass<type>& operator>>(type&);
    PointClass<type>& read(type&,int);
private:
    struct message_buf {
        long mtype;
        type info;
    } _buf;

};

#endif
