#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#include "queue.h"
#include "sem.h"

#define MAXLEN 16
#define BUFFER_SIZE 32
typedef struct mailbox
{
    int boxid;
    int ref_count; // the number of being referred
    char name[MAXLEN];
    int volume; // constant to buffer_size
    int wp;
    int rp;
    char buffer[BUFFER_SIZE];
    semaphore_t rsem;
    semaphore_t wsem;
} mailbox_t;

void mbox_init();
mailbox_t *mbox_open(char *);
void mbox_close(mailbox_t *);
void mbox_send(mailbox_t *, void *, int);
void mbox_recv(mailbox_t *, void *, int);

#endif