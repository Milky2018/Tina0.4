#include "sem.h"
#include "stdio.h"
#include "sched.h"

void do_semaphore_init(semaphore_t *s, int val)
{
    queue_init(&s->queue);
    do_mutex_lock_init(&s->lock);
    s->count = val;
}

void do_semaphore_up(semaphore_t *s)
{
    do_mutex_lock_acquire(&s->lock);
    s->count++;
    do_unblock_one(&s->queue);
    do_mutex_lock_release(&s->lock);
}

void do_semaphore_down(semaphore_t *s)
{
    do_mutex_lock_acquire(&s->lock);
    while (s->count <= 0) {
        do_mutex_lock_release(&s->lock);
        do_block(&s->queue);
        do_mutex_lock_acquire(&s->lock);
    }
    s->count--;
    do_mutex_lock_release(&s->lock);
}