#include "barrier.h"

void do_barrier_init(barrier_t *barrier, int goal)
{
    barrier->count = 0;
    barrier->max = goal;
    do_mutex_lock_init(&barrier->lock);
    queue_init(&barrier->queue);
}

void do_barrier_wait(barrier_t *barrier)
{
    do_mutex_lock_acquire(&barrier->lock);
    barrier->count++;
    if (barrier->count == barrier->max) {
        do_unblock_all(&barrier->queue);
        barrier->count = 0;
    } else {
        do_mutex_lock_release(&barrier->lock);
        do_block(&barrier->queue);
        do_mutex_lock_acquire(&barrier->lock);
    }
    do_mutex_lock_release(&barrier->lock);
}