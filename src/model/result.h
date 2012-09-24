#ifndef __result_h
#define __result_h

#include <string>

using namespace std;

class ResultClass {
public:
    ResultClass(string* status, string* value);
    string* value();
    string* status();
private:
    string* _status;
    string* _value;
};

#endif
