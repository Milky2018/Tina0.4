#include "test.h"
#include "stdio.h"
#include "syscall.h"
#include "proc.h"
#include "type.h"
#include "fs.h"


int hello_world()
{
    inode_entry_t a[2];
    inode_entry_t b[2];
    a[1].ptr[0] = 0x8765;
    a[1].ino = 0x10;
    a[0].ptr[0] = 0xabcd;
    a[0].ino = 0x11;
    inode_write(&a[0], 0x11);
    inode_write(&a[1], 0x10);
    inode_read(&b[0], 0x10);
    inode_read(&b[1], 0x11);
    sys_exit();
    return 0;
}