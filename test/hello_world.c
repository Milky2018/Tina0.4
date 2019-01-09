#include "test.h"
#include "stdio.h"
#include "syscall.h"
#include "proc.h"
#include "type.h"
#include "fs.h"

int hello_world()
{
    printf("%d\n", fd_index);
    sys_exit();
    return 0;
}