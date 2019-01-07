/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
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

#include "irq.h"
#include "test.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "screen.h"
#include "common.h"
#include "syscall.h"
#include "sync.h"
#include "proc.h"
#include "mailbox.h"
#include "string.h"
#include "mm.h"
#include "mac.h"

static void init_pcb()
{
	int i;
	queue_init(&ready_queue);
	assignt(&shell, 0);
	for (i = 1; i < NUM_MAX_TASK; i++) {
		pcb[i].status = TASK_EXITED;
	}
	current_running = queue_dequeue(&ready_queue);
}

static void init_page_table()
{
	unsigned int i;
	for (i = 0; i < PAGE_ENTRY_NUMBER; i++) {
		page_entrys[i].pfn = 0x0; // invalid
	}
}

static void init_memory()
{
	init_page_table();
}

static void init_variables()
{
	time_elapsed = 0;
	process_id = 1;
}

static void init_exception_handler()
{
	memcpy((void *)0x80000180, exception_handler_entry, exception_handler_end - exception_handler_begin);
	memcpy((void *)0x80000000, TLBexception_handler_entry, TLBexception_handler_end - TLBexception_handler_begin);
}

static void init_exception()
{
	init_exception_handler();
}

static void init_syscall(void)
{
	syscall[SYSCALL_SLEEP] = do_sleep;
	syscall[SYSCALL_SPAWN] = spawn;
	syscall[SYSCALL_KILL] = kill;
	syscall[SYSCALL_WAIT] = wait;
	syscall[SYSCALL_EXIT] = exit;
	syscall[SYSCALL_GETPID] = getpid;
	syscall[SYSCALL_BLOCK] = do_block;
	syscall[SYSCALL_UNBLOCK_ONE] = do_unblock_one;
	syscall[SYSCALL_UNBLOCK_ALL] = do_unblock_all;
	syscall[SYSCALL_WRITE] = screen_write;
	syscall[SYSCALL_CURSOR] = screen_move_cursor;
	syscall[SYSCALL_REFLUSH] = screen_reflush;
	syscall[SYSCALL_CLEAR] = command_clear;
	syscall[SYSCALL_MUTEX_LOCK_INIT] = do_mutex_lock_init;
	syscall[SYSCALL_MUTEX_LOCK_ACQUIRE] = do_mutex_lock_acquire;
	syscall[SYSCALL_MUTEX_LOCK_RELEASE] = do_mutex_lock_release;
	syscall[SYSCALL_COND_INIT] = do_condition_init;
	syscall[SYSCALL_COND_WAIT] = do_condition_wait;
	syscall[SYSCALL_COND_SIGNAL] = do_condition_signal;
	syscall[SYSCALL_COND_BROADCAST] = do_condition_broadcast;
	syscall[SYSCALL_SEM_INIT] = do_semaphore_init;
	syscall[SYSCALL_SEM_DOWN] = do_semaphore_down;
	syscall[SYSCALL_SEM_UP] = do_semaphore_up;
	syscall[SYSCALL_BARRIER_INIT] = do_barrier_init;
	syscall[SYSCALL_BARRIER_WATI] = do_barrier_wait;
	syscall[SYSCALL_INIT_MAC] = do_init_mac;
	syscall[SYSCALL_NET_RECV] = do_net_recv;
	syscall[SYSCALL_NET_SEND] = do_net_send;
	syscall[SYSCALL_WAIT_RECV_PACKAGE] = do_wait_recv_package;
}

void __attribute__((section(".entry_function"))) _start(void)
{
	asm_start();

	printk("Hello, Tina 0.2\n");

	init_variables();
	// printk("> [INIT] Global variables initialization succeeded\n");

	init_exception();
	// printk("> [INIT] Interrupt processing initialization succeeded.\n");

	init_memory();

	init_syscall();
	// printk("> [INIT] System call initialized successfully.\n");

	init_pcb();
	// printk("> [INIT] PCB initialization succeeded.\n");
	
	init_screen();
	// printk("> [INIT] SCREEN initialization succeeded.\n");

	fire();

	while(1)
		test_shell();

	return;
}

void printint(int a)
{
	printk("%x\n", a);
}