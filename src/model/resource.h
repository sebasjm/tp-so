#ifndef __resource_h
#define __resource_h

#include <string>

using namespace std;

class ResourceClass {
public:
    ResourceClass(const string nombre, const string value);
    const string nombre();
    const string value();
private:
    const string _nombre;
    const string _value;
};

#endif
