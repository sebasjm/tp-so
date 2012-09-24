#include <node/task-executer/taskExecuter.h>
#include <unistd.h>
#include <stdio.h>

#include <util/log_ansi.h>
#define modulo "testTaskExecuter"
__init_logname(modulo)

char* getStatus(int status) {
	switch (status) {
	case 1:
		return "Success";
	case 2:
		return "Complete";
	case 3:
		return "Fail";
	default:
		return "No existe";
	}
}

void functionReturn(TaskResponse *taskResult) {
	debug(modulo, "CallFunction: %d - %d(%s) - %s\n", taskResult->handle.thread, taskResult->status, getStatus(taskResult->status), taskResult->value);
        printf("%s\n", getStatus(taskResult->status) );
}

int main(int argc, char** argv) {
	CALLBACK_FUNCTION func = &functionReturn;
	TaskRun task3 = { "","","c0a7ae7d513f4beb2bc203d6f339f1b5", "AAAAAA", "ZZZZZZ", 1, 6, 100000, 10000, 100 };
	Handle handle = taskExecuter_executeTask(&task3, &func);

	TaskRun task = { "","","f91698cbd73bdcfad2912219940e9b05", "AAAAAA", "BBBBBB", 1, 6, 100000, 10000, 100 };
	taskExecuter_executeTask(&task, &func);

	TaskRun task2 = { "","","79caca6ff5b1b9fa6e6eb15d7557daaf", "AAAAAA", "AAABBB", 1, 6, 100000, 10000, 100 };
	taskExecuter_executeTask(&task2, &func);

	TaskRun task6 = { "","","7d5ed3d8bdbb3cc624b93e0beaf44501", "AAAAAA", "zzzzzz", 1, 6, 100000, 10000, 10 };
	taskExecuter_executeTask(&task6, &func);

	TaskRun task4 = { "","","e28dd5f3418351646bacff967b216860", "000000", "AAABBB", 1, 6, 100000, 10000, 100 };
	taskExecuter_executeTask(&task4, &func);

	TaskRun task5 = { "","","7d5ed3d8bdbb3cc624b93e0beaf44501", "AAyZ00", "zzzzzz", 1, 6, 100000, 10000, 100 };
	taskExecuter_executeTask(&task5, &func);

	taskExecuter_cancelTask(handle);
	sleep(8);
	taskExecuter_shutdown();
}

