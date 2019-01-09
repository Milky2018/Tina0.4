#ifndef INCLUDE_TEST_H
#define INCLUDE_TEST_H

#include "sched.h"

extern mutex_lock_t console;
extern struct task_info *test_tasks[16];
extern struct task_info shell;
int atoi(char *s);
int hello_world(void);
void test_fs(void);
void edit(void);

#endif
