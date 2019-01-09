#include "string.h"
#include "fs.h"

void touch_file(int argc, char *argv[])
{
    char *file_name;
    dir_block_t db;
    int ino;
    int i;
    if (argc < 2) 
        return;
    else
        file_name = argv[1];
    dir_block_read(&db, shell_path_inode.ino);

    for (i = 0; i < 0x200; i++) {
        if (db.dentry[i].ino == -1)
            break;
    }

    ino = inode_alloc();
    inode_create(ino, file, file_name, shell_path_inode.ino);
    db.dentry[i].ino = ino;
    db.dentry[i].mode = file;
    strcpy(db.dentry[i].name, file_name);

    dir_block_write(&db, shell_path_inode.ino);
}

void cat_file(int argc, char *argv[])
{
    char *file_name;
    dir_block_t db;
    file_block_t fb;
    int i;
    if (argc < 2) 
        return;
    else
        file_name = argv[1];
    dir_block_read(&db, shell_path_inode.ino);

    for (i = 0; i < 0x200; i++) {
        if (db.dentry[i].ino != -1 && db.dentry[i].mode == file && strcmp(db.dentry[i].name, file_name) == 0)
            break;
    }
    if (i == 0x200) {
        printf("error: no such file\n");
        return;
    }
    file_block_read(&fb, db.dentry[i].ino);
    printf("%s", (char *)&fb);
}

int sys_fopen(char *name, int access)
{
    int i;
    char *args[2] = {"touch", name};
    dir_block_t db;
    dir_block_read(&db, shell_path_inode.ino);

    for (i = 0; i < 0x200; i++) {
        if (db.dentry[i].ino != -1 && db.dentry[i].mode == file && strcmp(db.dentry[i].name, name) == 0) {
            fd[fd_index].ino = db.dentry[i].ino;
            fd[fd_index].pos = 0;
            fd_index++;
            return fd_index - 1;
        }
    }
    
    touch_file(2, args);
    dir_block_read(&db, shell_path_inode.ino);
    for (i = 0; i < 0x200; i++) {
        if (db.dentry[i].ino != -1 && db.dentry[i].mode == file && strcmp(db.dentry[i].name, name) == 0) {
            fd[fd_index].ino = db.dentry[i].ino;
            fd[fd_index].pos = 0;
            fd_index++;
            return fd_index - 1;
        }
    }
    printf("open failed\n");
    return 0;
}

int sys_fread(int index, char *buf, int size)
{
    file_block_t fb;

// printf("read 1\n");
    file_block_read(&fb, fd[index].ino);
// printf("read 2\n");
    memcpy(buf, (uint8_t *)&fb + fd[index].pos, size);
// printf("read 3\n");
    fd[index].pos += size;
    return size;
}

int sys_fwrite(int index, char *buf, int size)
{
    file_block_t fb;

// printf("write 1\n");
    file_block_read(&fb, fd[index].ino);
// printf("write 2\n");
    memcpy((uint8_t *)&fb + fd[index].pos, buf, size);
// printf("write 3\n");
    file_block_write(&fb, fd[index].ino);
// printf("write 4\n");
    fd[index].pos += size;
    return size;
}

void sys_fclose(int index)
{
    sys_fwrite(index, "", 1);
    fd[index].pos = 0;
}

void sys_fseek(int index, int pos)
{
    fd[index].pos = pos;
}