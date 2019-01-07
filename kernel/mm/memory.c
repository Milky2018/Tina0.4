#include "mm.h"
#include "proc.h"
#include "sched.h"

PageEntry_t page_entrys[PAGE_ENTRY_NUMBER];

void do_TLB_Refill()
{
    short coherency = 2;
    short dirt = 1;
    short valid = 1;
    short global = GLOBAL_ASID;
    unsigned int badvaddr = get_cp0_badvaddr();
    unsigned int vpn = badvaddr >> 12;
    unsigned int pfn0, pfn1, vpn2;
    // printk("here dealing Refill\n");
    // printk("badvaddr is 0x%x\n", badvaddr);
    vpn2 = vpn >> 1;
    if ((page_entrys[vpn2 * 2].pfn >> 28) != PTE_VALID)
        do_page_fault(vpn2 * 2);
    if ((page_entrys[vpn2 * 2 + 1].pfn >> 28) != PTE_VALID)
        do_page_fault(vpn2 * 2 + 1);
    pfn0 = page_entrys[vpn2 * 2].pfn & 0x1fff;
    pfn1 = page_entrys[vpn2 * 2 + 1].pfn & 0x1fff;
    set_cp0_entryhi(vpn2 << 13 | (current_running->pid & 0xff));
    set_cp0_entrylo0(pfn0 << 6 | coherency << 3 | dirt << 2 | valid << 1 | global);
    set_cp0_entrylo1(pfn1 << 6 | coherency << 3 | dirt << 2 | valid << 1 | global);
    set_cp0_pagemask(0);
    asm_tlbwr();
    // printk("                        The pair is VPN: 0x%x, 0x%x\n                        PFN: 0x%x, 0x%x\n", vpn2 * 2, vpn2 * 2 + 1, pfn0, pfn1);
    // printk("end dealing Refill\n");
}

void do_page_fault(int vpn)
{
    static int count = 0;
    // printk("                              %d\n", count);
    if (count > 0xfff)
        printk("physical memory is full!!!\n");
    page_entrys[vpn].pfn = 0xb0001000 + count++;
}