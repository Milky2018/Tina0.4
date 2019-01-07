#include "sched.h"
#include "stdio.h"
#include "syscall.h"
#include "time.h"
#include "screen.h"
#include "test.h"

int rand()
{	
	int current_time = get_ticks();
	return current_time % 100000;
}

unsigned long atox(char *s)
{
    long long n;
	int i;
	int sign;

    i = 0;
    n = 0;
    sign = (s[i] == '-') ? -1 : 1;
    if (s[i] == '+' || s[i] == '-')
        i++;
    while (s[i]) {
		if (s[i] >= '0' && s[i] <= '9') {
			n = 16 * n + s[i] - '0';
		} else if (s[i] >= 'a' && s[i] <= 'f') {
			n = 16 * n + s[i] - 'a' + 10;
		} else if (s[i] >= 'A' && s[i] <= 'F') {
			n = 16 * n + s[i] - 'A' + 10;
		} else {
			n = 16 * n;
		}
        i++;
    }

    return sign * n;
}

static void scanf(int *mem)
{

}

void setInteger(unsigned long *mem)
{
	char buf[128] = "";
    unsigned long i = 0;
	mutex_lock_acquire(&console);
    while (1) {
        disable_interrupt();
        char ch = read_uart_ch();
        enable_interrupt();
        
        switch(ch) {
            case '\0':
                break;
            case '\b':
            case 127:
                if (i) {
                    printf("\b");
                    i--;
                }
                break;
            case '\r':
            case '\n':
                printf("\n");
                buf[i++] = '\0';
                goto finish;
                i = 0;
                break;
            default:
                buf[i++] = ch;
                printf("%c", ch);
                break;
        }
        buf[i+1] = '\0';
    }
	finish:
	mutex_lock_release(&console);
	i = atox(buf);
	*mem = i;
}

void rw_task1(void)
{
	unsigned int mem1 = 0, mem2 = 0, mem3 = 0;
	int curs = 0;
	int i = 0;
	sys_move_cursor(0, curs + i);
	for (i = 0; i < 4; i++) {
		setInteger(&mem1);
		mem2 = rand();
		*(unsigned int *)mem1 = mem2;
		printf("write: 0x%x, %d\n", mem1, mem2);
        mem3 = *(unsigned int *)mem1;
        printf("load: 0x%x, %d\n", mem1, mem3);
	}

	sys_exit();
}
