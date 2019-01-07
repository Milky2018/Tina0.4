/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode. 
 *       The main function is to make system calls through the user's output.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include "test.h"
#include "stdio.h"
#include "screen.h"
#include "syscall.h"
#include "sched.h"
#include "string.h"
#include "proc.h"
#include "lock.h"
#include "fs.h"

#define CMD_LEN 128
#define CMD_MAX_COUNT 16

mutex_lock_t console;

char read_uart_ch(void)
{
    char ch = 0;
    unsigned char *read_port = (unsigned char *)(0xbfe48000 + 0x00);
    unsigned char *stat_port = (unsigned char *)(0xbfe48000 + 0x05);

    while ((*stat_port & 0x01))
    {
        ch = *read_port;
    }
    return ch;
}

struct task_info task1 = {"task1", (uint32_t)&phy_regs_task1, USER_PROCESS};
struct task_info task2 = {"task2", (uint32_t)&phy_regs_task2, USER_PROCESS};
struct task_info task3 = {"task3", (uint32_t)&phy_regs_task3, USER_PROCESS};

struct task_info task0 = {"hello", (uint32_t)&hello_world, USER_PROCESS};

struct task_info *test_tasks[16] = {&task0, &task1, &task2, &task3};

static int num_test_tasks = 2;

int cmd_broaden_location = 20;

void exec(int argc, char *argv[]);
void killer(int argc, char *argv[]);

int print_location = 21;
char cmd_head[] = "> root@Tina:";
char cmd_broaden[] = "--------------- COMMAND ---------------";

void process_show()
{
    int i;
    for (i = 0; i < NUM_MAX_TASK; i++) {
        if (pcb[i].status != TASK_EXITED) {
            printf("PROCESS[%d]: ", i);
            printf("name: %s   ", pcb[i].name);
            printf("pid: %d   ", pcb[i].pid);
            printf("status: ");
            switch (pcb[i].status) {
                case 0: 
                    printf("embryo\n");
                    break;
                case 1:
                    printf("sleeping\n");
                    break;
                case 2:
                    printf("blocked\n");
                    break;
                case 3:
                    printf("running\n");
                    break;
                case 4:
                    printf("ready\n");
                    break;
                case 5:
                    printf("exited\n");
                    break;
                default:
                    printf("error\n");
                    break;
            }
        }
    }
}

static void decode(char *origin_cmd)
{
    char *args[CMD_MAX_COUNT];
    int cmd_count = 0;
    int i = 0;
    char cmd[CMD_LEN];
    strcpy(cmd, origin_cmd);

    while (cmd[i]) {
        if (cmd[i] == ' ') {
            i++;
            continue;
        } else {
            args[cmd_count++] = cmd + i;
            i++;
            while (cmd[i]) {
                if (cmd[i] == ' ') {
                    cmd[i] = '\0';
                    i++;
                    break;
                } else {
                    i++;
                }
            }
        }
    }

    if (strcmp(args[0], "ps") == 0) {
        process_show();
    } else if (strcmp(args[0], "clear") == 0) {
        sys_clear();
        sys_move_cursor(1, cmd_broaden_location);
        printf("%s\n", cmd_broaden);
    } else if (strcmp(args[0], "exec") == 0) {
        printf("command[exec] executed\n");
        exec(cmd_count, args);
    } else if (strcmp(args[0], "kill") == 0) {
        printf("command[kill] executed\n");
        killer(cmd_count, args);
    } else if (strcmp(args[0], "mkfs") == 0) {
        printf("command[mkfs] executed\n");
        do_makefs();
    } else if (strcmp(args[0], "statfs") == 0) {
        statfs();
    } else if (strcmp(args[0], "delfs") == 0) {
        deletefs();
        printf("command[delfs] executed\n");
    } else if (strcmp(args[0], "mkdir") == 0) {
        make_directory(args[1]);
    } else if (strcmp(args[0], "ls") == 0) {
        list_directory(args[1]);
    } else if (strcmp(args[0], "cd") == 0) {
        change_directory(args[1]);
    } else {
        printf("error: invalid command: %s\n", args[0]);
    }
}

void exec(int argc, char *argv[])
{
    int count = 0;
    int i = 0;
    if (argc < 2) {
        printf("error: please input exec command with arguments, like \"all\"\n");
        return;
    }
    if (strcmp(argv[1], "all") == 0) {
        for (i = 0; i < 15; i++) {
            sys_spawn(test_tasks[i]);
        }
    } else {
        for (count = 1; count < argc; count++) {
            i = atoi(argv[count]);
            if (i < 16 && i >= 0)
                sys_spawn(test_tasks[i]);
        }
    }
}

void killer(int argc, char *argv[])
{
    int count = 0;
    int i = 0;
    if (argc < 2) {
        printf("error: please input kill command with arguments, like \"all\"\n");
        return;
    }
    if (strcmp(argv[1], "all") == 0) {
        for (i = 1; i < NUM_MAX_TASK; i++) {
            kill(pcb[i].pid);
        }
    } else {
        for (count = 1; count < argc; count++) {
            i = atoi(argv[count]);
            if (i < 16 && i >= 0)
                kill(i);
        }
    }
}

int atoi(char *s)
{
    int i, n, sign;

    i = 0;
    n = 0;
    sign = (s[i] == '-') ? -1 : 1;
    if (s[i] == '+' || s[i] == '-')
        i++;
    while (s[i]) {
        n = 10 * n + (s[i] - '0');
        i++;
    }

    return sign * n;
}

void test_shell()
{
    char buf[CMD_LEN] = "";
    int i = 0;
    mutex_lock_init(&console);
    sys_move_cursor(1, cmd_broaden_location);
    printf("%s\n", cmd_broaden);
    printf("%s%s%s", cmd_head, shell_path_head, "$ ");
    while (1) {
        mutex_lock_acquire(&console);
        disable_interrupt();
        char ch = read_uart_ch();
        enable_interrupt();
        mutex_lock_release(&console);
        
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
                decode(buf);
                i = 0;
                printf("%s%s%s", cmd_head, shell_path_head, "$ ");
                break;
            default:
                buf[i++] = ch;
                printf("%c", ch);
                break;
        }
        buf[i+1] = '\0';
    }
}

struct task_info shell = {"Shell", (uint32_t)&test_shell, USER_PROCESS};