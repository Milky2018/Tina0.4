#ifndef INCLUDE_SEM_H_
#define INCLUDE_SEM_H_

#include "queue.h"
#include "type.h"
#include "lock.h"

typedef struct semaphore
{
    queue_t queue;
    int count;
    mutex_lock_t lock;
} semaphore_t;

void do_semaphore_init(semaphore_t *, int);
void do_semaphore_up(semaphore_t *);
void do_semaphore_down(semaphore_t *);

#endif