#ifndef __dependency_h
#define __dependency_h

#include <string>

using namespace std;

class DependencyClass {
public:
    DependencyClass(const string nombre);
    const string nombre();
private:
    const string _nombre;
};

#endif
