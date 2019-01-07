#ifndef INCLUDE_MM_H_
#define INCLUDE_MM_H_

#include "type.h"

#define TLB_ENTRY_NUMBER 32
#define PAGE_ENTRY_NUMBER 0x2000

typedef struct PageEntry {
    unsigned int pfn; // | valid     | block number | PFN      |
                      // | 31 ... 28 | 27 ...... 13 | 12 ... 0 |
} PageEntry_t;

#define PTE_VALID 0xb

extern PageEntry_t page_entrys[PAGE_ENTRY_NUMBER];

void do_TLB_Refill();
void do_page_fault(int);

#endif
