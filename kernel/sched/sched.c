#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"
#include "irq.h"

pcb_t pcb[NUM_MAX_TASK];
pcb_t *current_running;
pid_t process_id = 1;
queue_t ready_queue;

static void check_sleeping()
{
    if (current_running->status == TASK_SLEEPING && get_timer() >= current_running->sleep_end){
        current_running->status = TASK_READY;
    }
}

void scheduler(uint32_t arg1, task_status_t status)
{
    queue_t *queue = (void *)arg1;
    uint32_t sleep_time = arg1;
    mutex_lock_t *lk;
    switch(status){
        case TASK_BLOCKED:
            current_running->status = TASK_BLOCKED;
            current_running->chan = queue;
            queue_push(queue, current_running);
            break;
        case TASK_SLEEPING:
            current_running->status = TASK_SLEEPING;
            current_running->sleep_end = get_timer() + sleep_time;
            queue_push(&ready_queue, current_running);
            break;
        case TASK_READY:
            current_running->status = TASK_READY;
            queue_push(&ready_queue, current_running);
            break;
        case TASK_EXITED:
            current_running->status = TASK_EXITED;
            // printk("                  %s exited", current_running->name);
            do_unblock_all(&current_running->waiting);
            while (!queue_is_empty(&current_running->holding)) {
                lk = queue_dequeue(&current_running->holding);
                do_mutex_lock_release(lk);
            }
            break;
        default:
            current_running->status = TASK_READY;
            queue_push(&ready_queue, current_running);
            break;
    }
    current_running->cursor_x = screen_cursor_x;
    current_running->cursor_y = screen_cursor_y;
    do{
        current_running = queue_dequeue(&ready_queue);
        check_sleeping();
        switch (current_running->status) {
            case TASK_SLEEPING:
                queue_push(&ready_queue, current_running);
                break;
            case TASK_EMBRYO:
                current_running->status = TASK_READY;
                break;
            case TASK_EXITED:
                // printk("                  %s is killed", current_running->name);
                do_unblock_all(&current_running->waiting);
                while (!queue_is_empty(&current_running->holding)) {
                    lk = queue_dequeue(&current_running->holding);
                    do_mutex_lock_release(lk);
                }
                break;
            case TASK_READY:
                break;
            default:
                printk("error: scheduler\n");
        }
    } while (current_running->status != TASK_READY);
    screen_cursor_x = current_running->cursor_x;
    screen_cursor_y = current_running->cursor_y;
    current_running->status = TASK_RUNNING;
    set_timer();
    rst_clk();
}

void do_scheduler(void)
{
    run_sched();
}

void do_sleep(uint32_t sleep_time)
{
    sleep_sched(sleep_time);
}

void do_block(queue_t *queue)
{
    block_sched(queue);
}

void do_exit()
{
    exit_sched();
}

void do_unblock_one(queue_t *queue)
{
    pcb_t *q;
    if (!queue_is_empty(queue)) {
        q = queue_dequeue(queue);
        q->status = TASK_READY;
        q->chan = NULL;
        queue_push(&ready_queue, q);
    }
}

void do_unblock_all(queue_t *queue)
{
    pcb_t *q;
    while (!queue_is_empty(queue)) {
        q = queue_dequeue(queue);
        q->status = TASK_READY;
        q->chan = NULL;
        queue_push(&ready_queue, q);
    }
}

void disable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status &= 0xfffffffe;
    set_cp0_status(cp0_status);
}

void enable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status |= 0x01;
    set_cp0_status(cp0_status);
}