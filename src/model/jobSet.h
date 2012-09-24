#ifndef __jobSet_h
#define __jobSet_h

#include "job.h"

#include <list>

using namespace std;

class JobSetClass {
public:
    JobSetClass(list<JobClass*> jobs);
    list<JobClass*> jobs();
private:
    list<JobClass*> _jobs;
};

#endif
