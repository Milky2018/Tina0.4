#ifndef __MAC_SUP__
#define __MAC_SUP__
#include "queue.h"
#include "mac.h"

extern queue_t recv_block_queue;
extern uint32_t buffer[PSIZE];
#define EPT_ARP 0x0608 /* type: ARP */

static void init_data(uint32_t *addr);

#endif