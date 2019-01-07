#include "irq.h"
#include "time.h"
#include "sched.h"
#include "string.h"
#include "stdio.h"
#include "mm.h"
#include "syscall.h"

static void irq_timer()
{
    screen_reflush();
    do_scheduler();
}

void set_timer()
{
    uint32_t clk_cmp = get_cp0_count();
    time_elapsed += clk_cmp * 2 / 300;
}

void interrupt_helper(uint32_t status, uint32_t cause)
{
    uint32_t cos = (cause >> 8) & 0xff;
    if (cos & 0x08) {
        if (*(uint32_t *)INT1_SR_ADDR & 0x08)
            irq_mac(); 
    } else if (cos & 0x80)
        irq_timer();
    else
        printk("other interrupts, IP7~0 is: %x\n", cos);
}

void other_exception_handler()
{
    uint32_t cause = get_cp0_cause();
    printk("other exceptions, exception code is: %x\n", (cause & ExcCode) >> 2);
}

void exception_handler(int fn, int arg1, int arg2, int arg3)
{
    uint32_t cause = get_cp0_cause();
    uint32_t status = get_cp0_status();
    int exccode = (cause & ExcCode) >> 2;
    switch (exccode) {
        case INT:
            interrupt_helper(status, cause);
            break;
        case TLBL:
        case TLBS:
            do_TLB_Refill();
            break;
        case SYS:
            user_epc_plus4();
            system_call_helper(fn, arg1, arg2, arg3);
            break;
        default:
            other_exception_handler();
            break;
    }
}

void other_TLB_handler()
{
    uint32_t cause = get_cp0_cause();
    printk("other TLB exceptions, exception code is: %x\n", (cause & ExcCode) >> 2);
}

void TLB_handler(void)
{
    uint32_t cause = get_cp0_cause();
    uint32_t status = get_cp0_status();
    uint32_t context = get_cp0_context();
    int exccode = (cause & ExcCode) >> 2;
    // printk("TLB exceptions\n");
    switch (exccode) {
        case TLBL:
        case TLBS:
            do_TLB_Refill();
            break;
        default:
            other_TLB_handler();
            break;
    }
}