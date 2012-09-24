#ifndef __point_h
#define __point_h

#include <fstream>
#include <string>
#include <sys/ipc.h>


using namespace std;

template <typename type = void*>
class PointClass {
public:

    virtual PointClass<type>& operator<<(const type) = 0;
    virtual PointClass<type>& operator>>(type&) = 0;

    operator int();
    const int id();

    PointClass(int i);
    virtual ~PointClass();
protected:
    int _id;

};

#endif
