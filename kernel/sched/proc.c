#include "proc.h"
#include "sched.h"
#include "stdio.h"
#include "queue.h"
#include "irq.h"

#define toString(x) #x

void *spawn(task_info_t *task)
{
    int i;
    for (i = 0; i < NUM_MAX_TASK; i++) {
        if (pcb[i].status == TASK_EXITED) {
            assignt(task, i);
            return;
        }
    }
    printk("error: pcbs full!\n");
    return;
}

void *kill(pid_t id)
{
    int i;
    for (i = 0; i < NUM_MAX_TASK; i++) {
        if (pcb[i].pid == id && pcb[i].status != TASK_EXITED) {
            pcb[i].status = TASK_EXITED;
            if (pcb[i].chan) {
                queue_remove(pcb[i].chan, &pcb[i]);
                queue_push(&ready_queue, &pcb[i]);
            }
            return;
        }
    }
    // error: no process's pid is $id
}

void *wait(pid_t id)
{
    int i;
    for (i = 0; i < NUM_MAX_TASK; i++) {
        if (pcb[i].pid == id && pcb[i].status != TASK_EXITED) {
            if (current_running->pid == id) {
                printk("error: wait id equals to self id\n");
                return;
            }
            do_block(&pcb[i].waiting);
            return;
        }
    }
}

void *exit()
{
    do_exit();
}

pid_t getpid()
{
    return_int32(current_running->pid);
    return current_running->pid;
}

void assign_task(pcb_t *dest, task_info_t *src, uint32_t kstack_top, uint32_t ustack_top)
{
	queue_push(&ready_queue, dest);
    queue_init(&dest->holding);
    queue_init(&dest->waiting);

    dest->name = src->name;

	dest->kernel_stack_top = kstack_top;
	dest->user_stack_top = ustack_top;

	dest->pid = process_id++;
	dest->type = src->type;
	dest->status = TASK_EMBRYO;

	dest->kernel_context.regs[29] = kstack_top;
	dest->kernel_context.regs[31] = embryo_start;
	dest->kernel_context.cp0_epc = src->entry_point;
	dest->kernel_context.cp0_status = 0x0;

	dest->user_context.regs[29] = ustack_top;
	dest->user_context.regs[31] = src->entry_point;
	dest->user_context.cp0_epc = src->entry_point;
	dest->user_context.cp0_status = 0xff01;
}