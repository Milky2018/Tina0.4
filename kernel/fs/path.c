#include "fs.h"
#include "string.h"

inode_entry_t shell_path_inode;
char shell_path_head[128] = "";
inode_entry_t cur_dir;

void mount()
{
    inode_read(&shell_path_inode, superblock.root_dir_ino);
    strcpy(shell_path_head, shell_path_inode.name);
}

void init_cur_dir(char s)
{
    if (s == '/')
        inode_read(&cur_dir, superblock.root_dir_ino);
    else 
        inode_read(&cur_dir, shell_path_inode.ino);
}

void change_directory(char *dir_name)
{
    decode_path(dir_name);
    inode_read(&shell_path_inode, cur_dir.ino);
    strcpy(shell_path_head, shell_path_inode.name);
}

void list_directory(char *dir_name)
{
    int i;
    dentry_sector ds;
    // if (decode_path(dir_name) < 0) {
    //     printf("ls error! no such directory!\n");
    //     return;
    // }
    do_sdread(&ds, cur_dir.ptr[0] * SECTOR_SIZE, SECTOR_SIZE);
    for (i = 0; i < 16; i++) {
        if (ds.den[i].ino != -1) {
            printf("%s      ", ds.den[i].name);
        }
    }
    printf("\n");
}

void make_directory(char *dir_name)
{
    // check if the dir_name has character '/'
    // ... to be done
    dentry_sector ds;
    int ino;
    int i;
    do_sdread(&ds, shell_path_inode.ptr[0] * SECTOR_SIZE, SECTOR_SIZE);

    for (i = 0; i < 16; i++) {
        if (ds.den[i].ino == -1)
            break;
    }

    // printf("error 11\n");
    // if (strcmp(ds.den[i].name, dir_name) == 0) {
    //     printf("mkdir error! %s already exists", dir_name);
    //     return;
    // }
    ino = inode_alloc();
    inode_init(ino, path, dir_name, shell_path_inode.ino);
    ds.den[i].ino = ino;
    ds.den[i].mode = path;
    strcpy(ds.den[i].name, dir_name);

    // printf("error 00\n");

    do_sdwrite(&ds, shell_path_inode.ptr[0] * SECTOR_SIZE, SECTOR_SIZE);
}

void remove(char *dir_name)
{

}

int decode_path(char *dir_name)
{
    // A/B/C -> A B C
    char *dir_args[16];
    int dir_num = 0;
    char buf[32];
    int i = 0;
    int j = 0;
    dentry_sector ds;

    strcpy(buf, dir_name);

    while (buf[i]) {
        if (buf[i] == '/') {
            i++;
            continue;
        } else {
            dir_args[dir_num++] = buf + i;
            i++;
            while (buf[i]) {
                if (buf[i] == '/') {
                    buf[i] = '\0';
                    i++;
                    break;
                } else {
                    i++;
                }
            }
        }
    }

    init_cur_dir(buf[0]);
    if (dir_num == 0)
        return 1;
    
    for (i = 0; i < dir_num; i++) {
        do_sdread(&ds, cur_dir.ptr[0] * SECTOR_SIZE, SECTOR_SIZE);
        for (j = 0; j < 16; i++) {
            if (ds.den[j].ino < 0 || ds.den[j].mode != path)
                break;
            if (strcmp(ds.den[j].name, dir_args[i]) == 0) {
                inode_read(&cur_dir, ds.den[j].ino);
                break;
            }
        }
        if (j == 16) {
            // printf("error: no such directory");
            return -1;
        }
    }
    return 1;

}