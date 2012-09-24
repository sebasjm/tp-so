#ifndef __target_h
#define __target_h

#include <string>

using namespace std;

class TargetClass {
public:
    TargetClass(string hash, int length);
    string hash();
    string method();
    int length();
private:
    string _hash;
    string _method;
    int _length;
};

#endif
