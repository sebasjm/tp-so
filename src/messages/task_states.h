/*
 * task_states.h
 *
 *  Created on: 04/11/2010
 *      Author: pmarchesi
 */

#ifndef TASK_STATES_H_
#define TASK_STATES_H_

#define TASK_UNKNOWN 0
#define TASK_SUCCESS 1
#define TASK_COMPLETE 2
#define TASK_FAIL 3


#define TASK_NAME(i) (i==0?"-UNKNOWN-":(i==1?"-SUCCESS-":(i==2?"-COMPLETE-":(i==3?"-FAIL-":"-INVALID-"))))
#endif /* TASK_STATES_H_ */
