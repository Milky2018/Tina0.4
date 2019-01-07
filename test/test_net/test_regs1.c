#include "mac.h"
#include "irq.h"
#include "type.h"
#include "screen.h"
#include "syscall.h"
#include "sched.h"
#include "test4.h"

queue_t recv_block_queue;
desc_t *send_desc;
desc_t *receive_desc;
uint32_t recv_flag[PNUM];
uint32_t ch_flag;
uint32_t cnt = 1; //record the time of iqr_mac
//uint32_t buffer[PSIZE] = {0x00040045, 0x00000100, 0x5d911120, 0x0101a8c0, 0xfb0000e0, 0xe914e914, 0x00000801,0x45000400, 0x00010000, 0x2011915d, 0xc0a80101, 0xe00000fb, 0x14e914e9, 0x01080000};
uint32_t buffer[PSIZE] = {0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0004e914, 0x0000, 0x005e0001, 0x2300fb00, 0x84b7f28b, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0801e914, 0x0000};

/**
 * Clears all the pending interrupts.
 * If the Dma status register is read then all the interrupts gets cleared
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void clear_interrupt()
{
    uint32_t data;
    data = reg_read_32(0xbfe11000 + DmaStatus);
    reg_write_32(0xbfe11000 + DmaStatus, data);
}

static void send_desc_init(mac_t *mac)
{
    int i;

    // mac->psize = PSIZE * 4;
    // mac->pnum = PNUM;
    // mac->mac_addr = 0xbfe10000;
    // mac->dma_addr = 0xbfe11000;
    mac->saddr = SEND_ADDR;
    // mac->daddr = RECV_ADDR;
    mac->saddr_phy = SEND_ADDR_PHY;
    // mac->daddr_phy = RECV_ADDR_PHY;
    mac->td = DMA_send_desc;
    // mac->rd = DMA_recv_desc;
    mac->td_phy = (uint32_t)DMA_send_desc & 0x1fffffff;
    // mac->rd = DMA_recv_desc & 0x1fffffff;

    // printf("size is %x\n", mac->psize);

    for (i = 0; i < PNUM; i++) {
        DMA_send_desc[i].tdes0 = 0x00000000;
        DMA_send_desc[i].tdes2 = SEND_ADDR + i * (mac->psize);
        // printf("i=%d mac->psize=0x400 The tdes2 is: %x\n", i, DMA_send_desc[i].tdes2);
        if (i == PNUM - 1) {
            DMA_send_desc[i].tdes1 = 0x63000400;
            DMA_send_desc[i].tdes3 = (uint32_t)&DMA_send_desc[0] & 0x1fffffff;
        } else {
            DMA_send_desc[i].tdes1 = 0x61000400;
            DMA_send_desc[i].tdes3 = (uint32_t)&DMA_send_desc[i+1] & 0x1fffffff;
        }
    }
}

static void recv_desc_init(mac_t *mac)
{
    int i;

    // mac->psize = PSIZE * 4;
    // mac->pnum = PNUM;
    // mac->mac_addr = 0xbfe10000;
    // mac->dma_addr = 0xbfe11000;
    // mac->saddr = SEND_ADDR;
    mac->daddr = RECV_ADDR;
    // mac->saddr_phy = SEND_ADDR_PHY;
    mac->daddr_phy = RECV_ADDR_PHY;
    // mac->td = DMA_send_desc;
    mac->rd = DMA_recv_desc;
    // mac->td_phy = DMA_send_desc & 0x1fffffff;
    mac->rd_phy = (uint32_t)DMA_recv_desc & 0x1fffffff;

    for (i = 0; i < PNUM; i++) {
        DMA_recv_desc[i].tdes0 = 0x80000000;
        DMA_recv_desc[i].tdes2 = RECV_ADDR + i * mac->psize;
        if (i == PNUM - 1) {
            DMA_recv_desc[i].tdes1 = 0x03000400;
            DMA_recv_desc[i].tdes3 = (uint32_t)&DMA_recv_desc[0] & 0x1fffffff;
        } else {
            DMA_recv_desc[i].tdes1 = 0x01000400;
            DMA_recv_desc[i].tdes3 = (uint32_t)&DMA_recv_desc[i+1] & 0x1fffffff;
        }
    }
}



static void mii_dul_force(mac_t *mac)
{
    reg_write_32(mac->dma_addr, 0x0); //?s
                                       //   reg_write_32(mac->dma_addr, 0x400);
    uint32_t conf = 0xc800;            //0x0080cc00;

    // loopback, 100M
    reg_write_32(mac->mac_addr, reg_read_32(mac->mac_addr) | (conf) | (1 << 8));
    //enable recieve all
    reg_write_32(mac->mac_addr + 0x4, reg_read_32(mac->mac_addr + 0x4) | 0x80000001);
}

void dma_control_init(mac_t *mac, uint32_t init_value)
{
    reg_write_32(mac->dma_addr + DmaControl, init_value);
    return;
}


void phy_regs_task1()
{

    mac_t test_mac;
    uint32_t i;
    uint32_t print_location = 2;

    test_mac.mac_addr = 0xbfe10000;
    test_mac.dma_addr = 0xbfe11000;

    test_mac.psize = PSIZE * 4; // 1024bytes
    test_mac.pnum = PNUM;       // pnum

    send_desc_init(&test_mac);

    dma_control_init(&test_mac, DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);
    clear_interrupt();

    mii_dul_force(&test_mac);

    sys_move_cursor(1, print_location);
    printf("> [SEND TASK] start send package.               \n");  
  
    uint32_t cnt = 0;
    i = 4;
    while (i > 0)
    {
        sys_net_send(test_mac.td, test_mac.td_phy);
        cnt += PNUM;
        sys_move_cursor(1, print_location);
        printf("> [SEND TASK] totally send package %d !        \n", cnt);
        i--;
    }
    sys_exit();
}

void phy_regs_task2()
{

    mac_t test_mac;
    uint32_t i;
    uint32_t ret;
    uint32_t print_location = 1;
    uint32_t cnt;
    uint32_t *Recv_desc;

    test_mac.mac_addr = 0xbfe10000;
    test_mac.dma_addr = 0xbfe11000;

    test_mac.psize = PSIZE * 4; // 64bytes
    test_mac.pnum = PNUM;       // pnum
    recv_desc_init(&test_mac);

    dma_control_init(&test_mac, DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);
    clear_interrupt();

    mii_dul_force(&test_mac);

    queue_init(&recv_block_queue);
    sys_move_cursor(1, print_location);
    printf("[RECV TASK] start recv:                    ");
    sys_move_cursor(1, print_location);
    while (1) {
        ret = sys_net_recv(test_mac.rd, test_mac.rd_phy, test_mac.daddr);
    
        Recv_desc = (uint32_t *)(test_mac.rd + (PNUM - 1) * 16);
        if (((*Recv_desc) & 0x80000000) == 0x80000000)
        {
            printf("> [RECV TASK] waiting receive package.\n");
            sys_wait_recv_package();
            clear_interrupt();
        }
        check_recv(&test_mac);
    }

    sys_exit();
}

void phy_regs_task3()
{
    uint32_t print_location = 1;
    sys_move_cursor(1, print_location);
    printf("> [INIT] Waiting for MAC initialization .\n");
    sys_init_mac();
    sys_move_cursor(1, print_location);
    printf("> [INIT] MAC initialization succeeded.           \n");
    // irq_enable(0);
    sys_exit();
}
