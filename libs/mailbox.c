#include "string.h"
#include "mailbox.h"
#include "sem.h"
#include "stdio.h"
#include "syscall.h"
#include "sched.h"

void disable_interrupt(void);
void enable_interrupt(void);

#define MAX_NUM_BOX 32

static mailbox_t mboxs[MAX_NUM_BOX];

int mboxinit = 0;

static void one_init(int i)
{
    mboxs[i].boxid = i;
    strcpy(mboxs[i].name, "$$$");
    mboxs[i].ref_count = 0;
    mboxs[i].volume = BUFFER_SIZE;
    mboxs[i].rp = 0;
    mboxs[i].wp = 0;
    mboxs[i].buffer[0] = 0xff;
    semaphore_init(&mboxs[i].rsem, 0);
    semaphore_init(&mboxs[i].wsem, BUFFER_SIZE);
}

void mbox_init()
{
    int i;
    for (i = 0; i < MAX_NUM_BOX; i++) {
        one_init(i);
    }
}

mailbox_t *mbox_create(char *name)
{
    int i;
    for (i = 0; i < MAX_NUM_BOX; i++) {
        if (mboxs[i].ref_count == 0 && strcmp(mboxs[i].name, "$$$") == 0) {
            one_init(i);
            strcpy(mboxs[i].name, name);
            mboxs[i].ref_count++;
            return &mboxs[i];
        }
    }
    printf("creating failed ");
    return NULL;
}

mailbox_t *mbox_destroy(int i)
{
    one_init(i);
}

mailbox_t *mbox_open(char *name)
{
    char tmp[16];
    int i;
    if (!mboxinit) {
        mbox_init();
        mboxinit++;
    }
    for (i = 0; i < 15 && name[i]; i++) {
        tmp[i] = name[i];
    }
    tmp[i] = '\0';
    for (i = 0; i < MAX_NUM_BOX; i++) {
        if (strcmp(tmp, mboxs[i].name) == 0) {
            mboxs[i].ref_count++;
            return &mboxs[i];
        }
    }
    return mbox_create(tmp);
}

void mbox_close(mailbox_t *mailbox)
{
    mailbox->ref_count--;
    if (mailbox->ref_count == 0) {
        mbox_destroy(mailbox->boxid);
    }
}

void mbox_send(mailbox_t *mailbox, void *msg, int msg_length)
{
    int i;
    char *_msg = (char *)msg;
    for (i = 0; i < msg_length; i++) {
        mailbox->wp %= BUFFER_SIZE;
        semaphore_down(&mailbox->wsem);
        disable_interrupt();
        mailbox->buffer[mailbox->wp++] = _msg[i];
        enable_interrupt();
        semaphore_up(&mailbox->rsem);
    }
}

void mbox_recv(mailbox_t *mailbox, void *msg, int msg_length)
{
    int i;
    char *_msg = (char *)msg;
    for (i = 0; i < msg_length; i++) {
        mailbox->rp %= BUFFER_SIZE;
        semaphore_down(&mailbox->rsem);
        disable_interrupt();
        _msg[i] = mailbox->buffer[mailbox->rp++];
        enable_interrupt();
        semaphore_up(&mailbox->wsem);
    }
}