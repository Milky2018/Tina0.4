#include "asm.h"

.equ    KERNEL, 0
.equ    USER,   KERNEL + OFFSET_SIZE
.equ    TIMECHICK, 0x10000
.equ    KERNEL_STACK_TOP, KERNEL + 2*OFFSET_SIZE

# next macro is for debugging
.macro PRINTINT inta
    li a0, \inta
    jal printint
.endm

.macro STI
    mfc0 t0, CP0_STATUS
    ori  t0, t0, 0x01
    mtc0 t0, CP0_STATUS
.endm

.macro CLI
    mfc0 t0, CP0_STATUS
    andi t0, t0, 0xfffe
    mtc0 t0, CP0_STATUS
.endm

.macro RESET_CLK
    li t0, TIMECHICK
    mtc0 t0, CP0_COMPARE
    mtc0 zero, CP0_COUNT
.endm

LEAF(asm_start)
    li      k0, 0xfc00
    mtc0    k0, CP0_STATUS # 1111 1100 0000 0000
    mtc0    zero, CP0_WATCHLO
    mtc0    zero, CP0_WATCHHI

    RESET_CLK

    mfc0    t0, CP0_CONFIG
    and     t0, ~0x7
    ori     t0, 0x2
    mtc0    t0, CP0_CONFIG
    
    jr  ra
END(asm_start)

.macro SAVE_CONTEXT offset
    lw k0, current_running(zero)
    sw $1 , OFFSET_REG1 + \offset(k0)
    sw $2 , OFFSET_REG2 + \offset(k0)
    sw $3 , OFFSET_REG3 + \offset(k0)
    sw $4 , OFFSET_REG4 + \offset(k0)
    sw $5 , OFFSET_REG5 + \offset(k0)
    sw $6 , OFFSET_REG6 + \offset(k0)
    sw $7 , OFFSET_REG7 + \offset(k0)
    sw $8 , OFFSET_REG8 + \offset(k0)
    sw $9 , OFFSET_REG9 + \offset(k0)
    sw $10, OFFSET_REG10 + \offset(k0)
    sw $11, OFFSET_REG11 + \offset(k0)
    sw $12, OFFSET_REG12 + \offset(k0)
    sw $13, OFFSET_REG13 + \offset(k0)
    sw $14, OFFSET_REG14 + \offset(k0)
    sw $15, OFFSET_REG15 + \offset(k0)
    sw $16, OFFSET_REG16 + \offset(k0)
    sw $17, OFFSET_REG17 + \offset(k0)
    sw $18, OFFSET_REG18 + \offset(k0)
    sw $19, OFFSET_REG19 + \offset(k0)
    sw $20, OFFSET_REG20 + \offset(k0)
    sw $21, OFFSET_REG21 + \offset(k0)
    sw $22, OFFSET_REG22 + \offset(k0)
    sw $23, OFFSET_REG23 + \offset(k0)
    sw $24, OFFSET_REG24 + \offset(k0)
    sw $25, OFFSET_REG25 + \offset(k0)
    sw $28, OFFSET_REG28 + \offset(k0)
    sw $29, OFFSET_REG29 + \offset(k0)
    sw $30, OFFSET_REG30 + \offset(k0)
    sw $31, OFFSET_REG31 + \offset(k0)
    mfc0 k1, CP0_STATUS
    sw k1, \offset + OFFSET_STATUS(k0)
    mfhi k1
    sw k1, \offset + OFFSET_HI(k0)
    mflo k1
    sw k1, \offset + OFFSET_LO(k0)
    mfc0 k1, CP0_EPC
    sw k1, \offset + OFFSET_EPC(k0)
    # mfc0 k1, CP0_ENTRYHI
    # sw k1, \offset + OFFSET_ENTRYHI(k0)
.endm

.macro RESTORE_CONTEXT offset
    lw k0, current_running(zero)
    lw $1 , OFFSET_REG1 + \offset(k0)
    lw $2 , OFFSET_REG2 + \offset(k0)
    lw $3 , OFFSET_REG3 + \offset(k0)
    lw $4 , OFFSET_REG4 + \offset(k0)
    lw $5 , OFFSET_REG5 + \offset(k0)
    lw $6 , OFFSET_REG6 + \offset(k0)
    lw $7 , OFFSET_REG7 + \offset(k0)
    lw $8 , OFFSET_REG8 + \offset(k0)
    lw $9 , OFFSET_REG9 + \offset(k0)
    lw $10, OFFSET_REG10 + \offset(k0)
    lw $11, OFFSET_REG11 + \offset(k0)
    lw $12, OFFSET_REG12 + \offset(k0)
    lw $13, OFFSET_REG13 + \offset(k0)
    lw $14, OFFSET_REG14 + \offset(k0)
    lw $15, OFFSET_REG15 + \offset(k0)
    lw $16, OFFSET_REG16 + \offset(k0)
    lw $17, OFFSET_REG17 + \offset(k0)
    lw $18, OFFSET_REG18 + \offset(k0)
    lw $19, OFFSET_REG19 + \offset(k0)
    lw $20, OFFSET_REG20 + \offset(k0)
    lw $21, OFFSET_REG21 + \offset(k0)
    lw $22, OFFSET_REG22 + \offset(k0)
    lw $23, OFFSET_REG23 + \offset(k0)
    lw $24, OFFSET_REG24 + \offset(k0)
    lw $25, OFFSET_REG25 + \offset(k0)
    lw $28, OFFSET_REG28 + \offset(k0)
    lw $29, OFFSET_REG29 + \offset(k0)
    lw $30, OFFSET_REG30 + \offset(k0)
    lw $31, OFFSET_REG31 + \offset(k0)
    lw k1, \offset + OFFSET_STATUS(k0)
    mtc0 k1, CP0_STATUS
    lw k1, \offset + OFFSET_HI(k0)
    mthi k1
    lw k1, \offset + OFFSET_LO(k0)
    mtlo k1
    lw k1, \offset + OFFSET_EPC(k0)
    mtc0 k1, CP0_EPC
    # lw k1, \offset + OFFSET_ENTRYHI(k0)
    # mtc0 k1, CP0_ENTRYHI
.endm

.macro SWTCH_TO_K
    lw k0, current_running(zero)
    lw sp, KERNEL_STACK_TOP(k0)
.endm

NESTED(run_sched, 0, ra)
    SAVE_CONTEXT(KERNEL)
    li      a1, 3
    jal     scheduler
    RESTORE_CONTEXT(KERNEL)
    jr      ra
END(run_sched)

NESTED(block_sched, 0, ra)
    SAVE_CONTEXT(KERNEL)
    li      a1, 2
    jal     scheduler
    RESTORE_CONTEXT(KERNEL)
    jr      ra
END(block_sched)

NESTED(sleep_sched, 0, ra)
    SAVE_CONTEXT(KERNEL)
    li      a1, 1
    jal     scheduler
    RESTORE_CONTEXT(KERNEL)
    jr      ra
END(sleep_sched)

NESTED(exit_sched, 0, ra)
    SAVE_CONTEXT(KERNEL)
    li      a1, 5
    jal     scheduler
    RESTORE_CONTEXT(KERNEL)
    jr      ra
END(exit_sched)

.global exception_handler_begin
.global exception_handler_end

NESTED(exception_handler_entry, 0, sp)   
exception_handler_begin:
    CLI
    SAVE_CONTEXT(USER)

    mfc0 k1, CP0_ENTRYHI
    sw k1, USER + OFFSET_ENTRYHI(k0)

    SWTCH_TO_K
    jal exception_handler
    RESTORE_CONTEXT(USER)

    lw k1, USER + OFFSET_ENTRYHI(k0)
    mtc0 k1, CP0_ENTRYHI

    STI
    eret
exception_handler_end:
END(exception_handler_entry)

.global TLBexception_handler_begin
.global TLBexception_handler_end

NESTED(TLBexception_handler_entry, 0, sp)
TLBexception_handler_begin:
    CLI
    SAVE_CONTEXT(USER)
    SWTCH_TO_K
    jal TLB_handler
    RESTORE_CONTEXT(USER)
    STI
    eret
TLBexception_handler_end:
END(TLBexception_handler_entry)

LEAF(get_cp0_status)
    mfc0 v0, CP0_STATUS
    jr ra
END(get_cp0_status)

LEAF(set_cp0_status)
    mtc0 a0, CP0_STATUS
    jr ra
END(set_cp0_status)

LEAF(get_cp0_cause)
    mfc0 v0, CP0_CAUSE
    jr ra
END(get_cp0_cause)

LEAF(get_cp0_epc)
    mfc0 v0, CP0_EPC
    jr ra
END(get_cp0_epc)

LEAF(set_cp0_epc)
    mtc0 a0, CP0_EPC
    jr ra
END(set_cp0_epc)

LEAF(get_cp0_compare)
    mfc0 v0, CP0_COMPARE
    jr ra
END(get_cp0_compare)

LEAF(get_cp0_count)
    mfc0 v0, CP0_COUNT
    jr ra
END(get_cp0_count)

LEAF(get_cp0_entrylo0)
    mfc0 v0, CP0_ENTRYLO0
    jr ra 
END(get_cp0_entrylo0)

LEAF(set_cp0_entrylo0)
    mtc0 a0, CP0_ENTRYLO0
    jr ra 
END(set_cp0_entrylo0)

LEAF(get_cp0_entrylo1)
    mfc0 v0, CP0_ENTRYLO1
    jr ra 
END(get_cp0_entrylo1)

LEAF(set_cp0_entrylo1)
    mtc0 a0, CP0_ENTRYLO1
    jr ra 
END(set_cp0_entrylo1)

LEAF(get_cp0_pagemask)
    mfc0 v0, CP0_PAGEMASK
    jr ra 
END(get_cp0_pagemask)

LEAF(set_cp0_pagemask)
    mtc0 a0, CP0_PAGEMASK
    jr ra 
END(set_cp0_pagemask)

LEAF(get_cp0_entryhi)
    mfc0 v0, CP0_ENTRYHI
    jr ra 
END(get_cp0_entryhi)

LEAF(set_cp0_entryhi)
    mtc0 a0, CP0_ENTRYHI
    jr ra 
END(set_cp0_entryhi)

LEAF(get_cp0_index)
    mfc0 v0, CP0_INDEX
    jr ra 
END(get_cp0_index)

LEAF(set_cp0_index)
    mtc0 a0, CP0_INDEX
    jr ra 
END(set_cp0_index)

LEAF(get_cp0_context)
    mfc0 v0, CP0_CONTEXT
    jr ra
END(get_cp0_context)

LEAF(set_cp0_context)
    mtc0 a0, CP0_CONTEXT
    jr ra
END(set_cp0_context)

LEAF(get_cp0_badvaddr)
    mfc0 v0, CP0_BADVADDR
    jr ra
END(get_cp0_badvaddr)

LEAF(asm_tlbwi)
    tlbwi 
    jr ra 
END(asm_tlbwi)

LEAF(asm_tlbwr)
    tlbwr
    jr ra 
END(asm_tlbwr)

LEAF(fire)
    RESET_CLK

    li k0, 0xff01
    mtc0 k0, CP0_STATUS

    jr ra
END(fire)

LEAF(embryo_start)
    RESTORE_CONTEXT(USER)
    STI
    eret
END(embryo_start)

LEAF(user_epc_plus4)
    lw k0, current_running(zero)
    lw k1, USER + OFFSET_EPC(k0)
    addi k1, k1, 0x4
    sw k1, USER + OFFSET_EPC(k0)
    jr ra
END(user_epc_plus4)

LEAF(rst_clk)
    RESET_CLK
    jr ra
END(rst_clk)

LEAF(return_int32)
    lw k0, current_running(zero)
    sw a0, OFFSET_REG2 + USER(k0)
    jr ra
END(return_int32)