#include "jobSet.h"

JobSetClass::JobSetClass(list<JobClass*> jobs) {
    this->_jobs = jobs;
}

list<JobClass*> JobSetClass::jobs(){
    return this->_jobs;
}
