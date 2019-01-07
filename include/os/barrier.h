#ifndef INCLUDE_BARRIER_H_
#define INCLUDE_BARRIER_H_

#include "queue.h"
#include "lock.h"

typedef struct barrier
{
    int count;
    int max;
    queue_t queue;
    mutex_lock_t lock;
} barrier_t;

void do_barrier_init(barrier_t *, int);
void do_barrier_wait(barrier_t *);

#endif