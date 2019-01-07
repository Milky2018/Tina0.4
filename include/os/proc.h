#ifndef __PROC__
#define __PROC__
#include "sched.h"

#define assignt(src, i) do {\
assign_task(&pcb[i], src, KSTACK_BASE + i*KSTACK_SIZE, USTACK_BASE + i*USTACK_SIZE);\
} while(0)

void assign_task(pcb_t *dest, task_info_t *src, uint32_t kstack_top, uint32_t ustack_top);

void *spawn(task_info_t *task);
void *kill(pid_t id);
void *exit(void);
void *wait(pid_t id);
pid_t getpid();

#endif