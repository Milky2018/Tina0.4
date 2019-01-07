#include "time.h"
#include "test3.h"
#include "lock.h"
#include "sched.h"
#include "stdio.h"
#include "syscall.h"

static char blank[] = {"                                                "};

mutex_lock_t lock1;
mutex_lock_t lock2;

// pid = 2
int task1_pid = 0;

void ready_to_exit_task()
{
    int i = 0, print_location = 0;

    task1_pid = current_running->pid;

    mutex_lock_acquire(&lock1);
    mutex_lock_acquire(&lock2);

    // sys_spawn(&task1);
    // sys_spawn(&task2);

    for (i = 0; i < 500; i++)
    {
        sys_move_cursor(0, print_location);
        printf("> [TASK] I am task with pid %d, I have acquired two mutex lock. (%d)", task1_pid, i++);
    }
    sys_exit(); // test exit
}

// pid = 3
void wait_lock_task()
{
    int i, print_location = 1;

    sys_move_cursor(0, print_location);
    printf("> [TASK] I want to acquire a mute lock from task(pid=%d).", task1_pid);

    mutex_lock_acquire(&lock1);

    sys_move_cursor(0, print_location);
    printf("> [TASK] I have acquired a mutex lock from task(pid=%d).", task1_pid);

    sys_exit(); // test exit
}

// pid = 4
void wait_exit_task()
{
    int i, print_location = 2;

    sys_move_cursor(0, print_location);
    printf("> [TASK] I want to wait task (pid=%d) to exit.", task1_pid);

    sys_waitpid(task1_pid); //test waitpid

    sys_move_cursor(0, print_location);
    printf("> [TASK] Task (pid=%d) has exited.                ", task1_pid);

    sys_exit(); // test exit
}