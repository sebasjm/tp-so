#ifndef __socket_h
#define __socket_h

#include <fstream>
#include <string>
#include <sys/ipc.h>
#include <arpa/inet.h>

#include "point.h"

using namespace std;

template <typename type = void*>
class SocketClass : public PointClass<type> {
public:
    SocketClass(const int id);
    ~SocketClass();

    PointClass<type>& operator<<(const type);
    PointClass<type>& operator>>(type&);

    bool isClosed();
    bool hasEnded();
    void stop();
private:
    bool _isClosed;
    bool _hasEnded;
};

sockaddr_in get_info(int sock);

#define str_ip(s) inet_ntoa(get_info(s).sin_addr)
#define int_puerto(s) ntohs(get_info(s).sin_port)

#endif
