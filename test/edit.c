#include "fs.h"
#include "string.h"
#include "test.h"
#include "syscall.h"
#include "stdio.h"

void disable_interrupt();
void enable_interrupt();
char read_uart_ch();

void edit(void)
{

    sys_exit();
}

void write_to_buf(void)
{
    char buf[SECTOR_SIZE];
    int i = 0;
    mutex_lock_acquire(&console);
    while (1) {
        disable_interrupt();
        char ch = read_uart_ch();
        enable_interrupt();

        switch(ch) {
            case '\0':
            case '\t':
                buf[i++] = '\0';
                goto finish;
            case 127:
                if (i) {
                    printf("\b");
                    i--;
                }
                break;
            default:
                buf[i++] = ch;
                printf("%c", ch);
                break;
        }
        buf[i + 1] = '\0';
    }
    finish:
    mutex_lock_release(&console);
}