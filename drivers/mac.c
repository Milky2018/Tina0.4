#include "mac.h"
#include "irq.h"
#include "string.h"
#include "mac_support.h"
#include "sched.h"

desc_t DMA_recv_desc[64];
desc_t DMA_send_desc[64];

// uint32_t buffer[PSIZE] = {
//                             0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 
//                             0x0000d400, 0x11ff0040, 0xa8c073d8, 0x00e00101, 
//                             0xe914fb00, 0x0004e914, 0x0000, 0x005e0001, 
//                             0x2300fb00, 0x84b7f28b, 0x00450008, 0x0000d400, 
//                             0x11ff0040, 0xa8c073d8, 0x00e00101, 0xe914fb00, 
//                             0x0801e914, 0x0000
//                          };

uint32_t reg_read_32(uint32_t addr)
{
    return *((uint32_t *)addr);
}
uint32_t read_register(uint32_t base, uint32_t offset)
{
    uint32_t addr = base + offset;
    uint32_t data;

    data = *(volatile uint32_t *)addr;
    return data;
}

void reg_write_32(uint32_t addr, uint32_t data)
{
    *((uint32_t *)addr) = data;
}

static void gmac_get_mac_addr(uint8_t *mac_addr)
{
    uint32_t addr;

    addr = read_register(GMAC_BASE_ADDR, GmacAddr0Low);
    mac_addr[0] = (addr >> 0) & 0x000000FF;
    mac_addr[1] = (addr >> 8) & 0x000000FF;
    mac_addr[2] = (addr >> 16) & 0x000000FF;
    mac_addr[3] = (addr >> 24) & 0x000000FF;

    addr = read_register(GMAC_BASE_ADDR, GmacAddr0High);
    mac_addr[4] = (addr >> 0) & 0x000000FF;
    mac_addr[5] = (addr >> 8) & 0x000000FF;
}

/* print DMA regs */
void print_dma_regs()
{
    uint32_t regs_val1, regs_val2;

    printk(">>[DMA Register]\n");

    // [0] Bus Mode Register
    regs_val1 = read_register(DMA_BASE_ADDR, DmaBusMode);

    printk("  [0] Bus Mode : 0x%x, ", regs_val1);

    // [3-4] RX/TX List Address Register
    regs_val1 = read_register(DMA_BASE_ADDR, DmaRxBaseAddr);
    regs_val2 = read_register(DMA_BASE_ADDR, DmaTxBaseAddr);
    printk("  [3-4] TX/RX : 0x%x/0x%x\n", regs_val2, regs_val1);

    // [5] Status Register
    regs_val1 = read_register(DMA_BASE_ADDR, DmaStatus);
    printk("  [5] Status : 0x%x, ", regs_val1);

    // [6] Operation Mode Register
    regs_val1 = read_register(DMA_BASE_ADDR, DmaControl);
    printk("  [6] Control : 0x%x\n", regs_val1);

    // [7] Interrupt Enable Register
    regs_val1 = read_register(DMA_BASE_ADDR, DmaInterrupt);
    printk("  [7] Interrupt : 0x%x, ", regs_val1);

    // [8] Miss
    regs_val1 = read_register(DMA_BASE_ADDR, DmaMissedFr);
    printk("  [8] Missed : 0x%x\n", regs_val1);

    // [18-19] Current Host TX/RX Description Register
    regs_val1 = read_register(DMA_BASE_ADDR, DmaTxCurrDesc);
    regs_val2 = read_register(DMA_BASE_ADDR, DmaRxCurrDesc);
    printk("  [18-19] Current Host TX/RX Description : 0x%x/0x%x\n", regs_val1, regs_val2);

    // [20-21] Current Host TX/RX Description Register
    regs_val1 = read_register(DMA_BASE_ADDR, DmaTxCurrAddr);
    regs_val2 = read_register(DMA_BASE_ADDR, DmaRxCurrAddr);
    printk("  [20-21] Current Host TX/RX Buffer Address : 0x%x/0x%x\n", regs_val1, regs_val2);
}

/* print DMA regs */
void print_mac_regs()
{
    printk(">>[MAC Register]\n");
    uint32_t regs_val1, regs_val2;
    uint8_t mac_addr[6];

    // [0] MAC Configure Register
    regs_val1 = read_register(GMAC_BASE_ADDR, GmacConfig);
    printk("  [0] Configure : 0x%x, ", regs_val1);

    // [1] MAC Frame Filter
    regs_val1 = read_register(GMAC_BASE_ADDR, GmacFrameFilter);
    printk("  [1] Frame Filter : 0x%x\n", regs_val1);

    // [2-3] Hash Table High/Low Register
    regs_val1 = read_register(GMAC_BASE_ADDR, GmacHashHigh);
    regs_val2 = read_register(GMAC_BASE_ADDR, GmacHashLow);
    printk("  [2-3] Hash Table High/Low : 0x%x-0x%x\n", regs_val1, regs_val2);

    // [6] Flow Control Register
    regs_val1 = read_register(GMAC_BASE_ADDR, GmacFlowControl);
    printk("  [6] Flow Control : 0x%x, ", regs_val1);

    // [8] Version Register
    regs_val1 = read_register(GMAC_BASE_ADDR, GmacVersion);
    printk("  [8] Version : 0x%x\n", regs_val1);

    // [14] Interrupt Status Register and Interrupt Mask
    regs_val1 = read_register(GMAC_BASE_ADDR, GmacInterruptStatus);
    regs_val2 = read_register(GMAC_BASE_ADDR, GmacInterruptMask);
    printk("  [14-15] Interrupt Status/Mask : 0x%x/0x%x\n", regs_val1, regs_val2);

    // MAC address
    gmac_get_mac_addr(mac_addr);
    printk("  [16-17] Mac Addr : %X:%X:%X:%X:%X:%X\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
}

void printf_dma_regs()
{
    uint32_t regs_val1, regs_val2;

    printf(">>[DMA Register]\n");

    // [0] Bus Mode Register
    regs_val1 = read_register(DMA_BASE_ADDR, DmaBusMode);

    printf("  [0] Bus Mode : 0x%x, ", regs_val1);

    // [3-4] RX/TX List Address Register
    regs_val1 = read_register(DMA_BASE_ADDR, DmaRxBaseAddr);
    regs_val2 = read_register(DMA_BASE_ADDR, DmaTxBaseAddr);
    printf("  [3-4] RX/TX : 0x%x/0x%x\n", regs_val1, regs_val2);

    // [5] Status Register
    regs_val1 = read_register(DMA_BASE_ADDR, DmaStatus);
    printf("  [5] Status : 0x%x, ", regs_val1);

    // [6] Operation Mode Register
    regs_val1 = read_register(DMA_BASE_ADDR, DmaControl);
    printf("  [6] Control : 0x%x\n", regs_val1);

    // [7] Interrupt Enable Register
    regs_val1 = read_register(DMA_BASE_ADDR, DmaInterrupt);
    printf("  [7] Interrupt : 0x%x, ", regs_val1);

    // [8] Miss
    regs_val1 = read_register(DMA_BASE_ADDR, DmaMissedFr);
    printf("  [8] Missed : 0x%x\n", regs_val1);

    // [18-19] Current Host TX/RX Description Register
    regs_val1 = read_register(DMA_BASE_ADDR, DmaTxCurrDesc);
    regs_val2 = read_register(DMA_BASE_ADDR, DmaRxCurrDesc);
    printf("  [18-19] Current Host TX/RX Description : 0x%x/0x%x\n", regs_val1, regs_val2);

    // [20-21] Current Host TX/RX Description Register
    regs_val1 = read_register(DMA_BASE_ADDR, DmaTxCurrAddr);
    regs_val2 = read_register(DMA_BASE_ADDR, DmaRxCurrAddr);
    printk("  [20-21] Current Host TX/RX Buffer Address : 0x%x/0x%x\n", regs_val1, regs_val2);
}

/* print DMA regs */
void printf_mac_regs()
{
    printf(">>[MAC Register]\n");
    uint32_t regs_val1, regs_val2;
    uint8_t mac_addr[6];

    // [0] MAC Configure Register
    regs_val1 = read_register(GMAC_BASE_ADDR, GmacConfig);
    printf("  [0] Configure : 0x%x, ", regs_val1);

    // [1] MAC Frame Filter
    regs_val1 = read_register(GMAC_BASE_ADDR, GmacFrameFilter);
    printf("  [1] Frame Filter : 0x%x\n", regs_val1);

    // [2-3] Hash Table High/Low Register
    regs_val1 = read_register(GMAC_BASE_ADDR, GmacHashHigh);
    regs_val2 = read_register(GMAC_BASE_ADDR, GmacHashLow);
    printf("  [2-3] Hash Table High/Low : 0x%x-0x%x\n", regs_val1, regs_val2);

    // [6] Flow Control Register
    regs_val1 = read_register(GMAC_BASE_ADDR, GmacFlowControl);
    printf("  [6] Flow Control : 0x%x, ", regs_val1);

    // [8] Version Register
    regs_val1 = read_register(GMAC_BASE_ADDR, GmacVersion);
    printf("  [8] Version : 0x%x\n", regs_val1);

    // [14] Interrupt Status Register and Interrupt Mask
    regs_val1 = read_register(GMAC_BASE_ADDR, GmacInterruptStatus);
    regs_val2 = read_register(GMAC_BASE_ADDR, GmacInterruptMask);
    printf("  [14-15] Interrupt Status/Mask : 0x%x/0x%x\n", regs_val1, regs_val2);

    // MAC address
    gmac_get_mac_addr(mac_addr);
    printf("  [16-17] Mac Addr : %X:%X:%X:%X:%X:%X\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
}

void print_tx_dscrb(mac_t *mac)
{
    uint32_t i;
    printf("send buffer mac->saddr=0x%x ", mac->saddr);
    printf("mac->saddr_phy=0x%x ", mac->saddr_phy);
    printf("send discrb mac->td_phy=0x%x\n", mac->td_phy);
#if 0
    desc_t *send=mac->td;
    for(i=0;i<mac->pnum;i++)
    {
        printf("send[%d].tdes0=0x%x ",i,send[i].tdes0);
        printf("send[%d].tdes1=0x%x ",i,send[i].tdes1);
        printf("send[%d].tdes2=0x%x ",i,send[i].tdes2);
        printf("send[%d].tdes3=0x%x ",i,send[i].tdes3);
    }
#endif
}

void print_rx_dscrb(mac_t *mac)
{
    uint32_t i;
    printf("recieve buffer add mac->daddr=0x%x ", mac->daddr);
    printf("mac->daddr_phy=0x%x ", mac->daddr_phy);
    printf("recieve discrb add mac->rd_phy=0x%x\n", mac->rd_phy);
    desc_t *recieve = (desc_t *)mac->rd;
#if 0
    for(i=0;i<mac->pnum;i++)
    {
        printf("recieve[%d].tdes0=0x%x ",i,recieve[i].tdes0);
        printf("recieve[%d].tdes1=0x%x ",i,recieve[i].tdes1);
        printf("recieve[%d].tdes2=0x%x ",i,recieve[i].tdes2);
        printf("recieve[%d].tdes3=0x%x\n",i,recieve[i].tdes3);
    }
#endif
}

void irq_mac(void)
{
    // static int i = 0;
    // while (DMA_recv_desc[i].tdes0 >> 31 == 0) {
    //     DMA_recv_desc[i].tdes0 = 0x80000000;
    //     i++;
    //     if (i == PNUM) {
    //         do_unblock_all(&recv_block_queue);
    //         clear_interrupt();
    //         i = 0;
    //         break;
    //     }
    // }
    if (DMA_recv_desc[63].tdes0 >> 31 == 0) {
        clear_interrupt();
        do_unblock_all(&recv_block_queue);
    }
}

void irq_enable(int IRQn)
{
    reg_write_32(INT1_EN_ADDR, reg_read_32(INT1_EN_ADDR) | 0x08);
}

void check_recv(mac_t *test_mac)
{
    static i = 0;
    int j;
    for (j = 0; j < PNUM; i++, j++) {
        printf("%d ", i);
        // for (j = 0; j < 4; j++)
        //     printf("%x ", *((uint32_t *)(DMA_recv_desc[i].tdes2 + 4 * j)));
    }
    printf("\n");
}

void set_sram_ctr()
{
    *((volatile unsigned int *)0xbfd00420) = 0x8000; /* 使能GMAC0 */
}
static void s_reset(mac_t *mac) //reset mac regs
{
    uint32_t time = 1000000;
    reg_write_32(mac->dma_addr, 0x01);

    while ((reg_read_32(mac->dma_addr) & 0x01))
    {
        reg_write_32(mac->dma_addr, 0x01);
        while (time)
        {
            time--;
        }
    };
}
void disable_interrupt_all(mac_t *mac)
{
    reg_write_32(mac->dma_addr + DmaInterrupt, DmaIntDisable);
    return;
}
void set_mac_addr(mac_t *mac)
{
    uint32_t data;
    uint8_t MacAddr[6] = {0x00, 0x55, 0x7b, 0xb5, 0x7d, 0xf7};
    uint32_t MacHigh = 0x40, MacLow = 0x44;
    data = (MacAddr[5] << 8) | MacAddr[4];
    reg_write_32(mac->mac_addr + MacHigh, data);
    data = (MacAddr[3] << 24) | (MacAddr[2] << 16) | (MacAddr[1] << 8) | MacAddr[0];
    reg_write_32(mac->mac_addr + MacLow, data);
}
uint32_t do_net_recv(uint32_t rd, uint32_t rd_phy, uint32_t daddr)
{
    int i = 0, j = 0;

    reg_write_32(DMA_BASE_ADDR + 0x0c, rd_phy);
    reg_write_32(GMAC_BASE_ADDR, reg_read_32(GMAC_BASE_ADDR) | 0x04);

    reg_write_32(DMA_BASE_ADDR + 0x18, reg_read_32(DMA_BASE_ADDR + 0x18) | 0x02200002); // start tx, rx
    reg_write_32(DMA_BASE_ADDR + 0x1c, 0x10001 | (1 << 6));

    // printk("this descriptor address: %x\n next descriptor address: %x\n", &DMA_recv_desc[0], DMA_recv_desc[0].tdes3);

    for (i = 0; i < PNUM; i++) {
        DMA_recv_desc[i].tdes0 |= 0x80000000;
        reg_write_32(DMA_BASE_ADDR + 0x8, 0x01);
    }
    irq_enable(0);

    return_int32(0);
    return 0;
}

void do_net_send(uint32_t td, uint32_t td_phy)
{
    int i = 0;

    reg_write_32(DMA_BASE_ADDR + 0x10, td_phy);
    reg_write_32(GMAC_BASE_ADDR, reg_read_32(GMAC_BASE_ADDR) | 0x08);
    
    reg_write_32(DMA_BASE_ADDR + 0x18, reg_read_32(DMA_BASE_ADDR + 0x18) | 0x02202000); //0x02202002); // start tx, rx
    reg_write_32(DMA_BASE_ADDR + 0x1c, 0x10001 | (1 << 6));

    for (i = 0; i < PNUM; i++) {
        memcpy(DMA_send_desc[i].tdes2, buffer, PSIZE * sizeof(uint32_t));
        DMA_send_desc[i].tdes0 |= 0x80000000;
        reg_write_32(DMA_BASE_ADDR + 0x4, 0x01);
        while (DMA_send_desc[i].tdes0 >> 31);
    }
}

void do_init_mac(void)
{
    mac_t test_mac;
    uint32_t i;

    test_mac.mac_addr = 0xbfe10000;
    test_mac.dma_addr = 0xbfe11000;

    test_mac.psize = PSIZE * 4; // 64bytes
    test_mac.pnum = PNUM;       // pnum

    set_sram_ctr(); /* 使能GMAC0 */
    s_reset(&test_mac);
    disable_interrupt_all(&test_mac);
    set_mac_addr(&test_mac);
    reg_write_32(INT1_CLR_ADDR, 0xffffffff);
    reg_write_32(INT1_POL_ADDR, 0xffffffff);
    reg_write_32(INT1_EDGE_ADDR, 0x0);
    // irq_enable(0);
}

void do_wait_recv_package(void)
{
    do_block(&recv_block_queue);
}