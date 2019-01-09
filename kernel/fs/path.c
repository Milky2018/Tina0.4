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

static void init_cur_dir(char s)
{
    if (s == '/')
        inode_read(&cur_dir, superblock.root_dir_ino);
    else 
        inode_read(&cur_dir, shell_path_inode.ino);
}

void change_directory(int argc, char *argv[])
{
    char *dir_name;
    if (argc < 2) 
        strcpy(dir_name, ".");
    else
        dir_name = argv[1];
    if (decode_path(dir_name) < 0)
        return;
    inode_read(&shell_path_inode, cur_dir.ino);
    strcpy(shell_path_head, shell_path_inode.name);
}

void list_directory(int argc, char *argv[])
{
    char *dir_name;
    int i;
    dir_block_t db;
    if (argc < 2) 
        strcpy(dir_name, ".");
    else
        dir_name = argv[1];
    if (decode_path(dir_name) < 0)
        return;
    dir_block_read(&db, cur_dir.ino);
    for (i = 0; i < DENTRYS_PER_BLOCK; i++) {
        if (db.dentry[i].ino == -1) 
            continue;
        // else if (bitmap_getbit(&inode_map, db.dentry[i].ino) == 0)
        //     continue;
        else 
            printf("%s    ", db.dentry[i].name);
    }
    printf("\n");
}

void make_directory(int argc, char *argv[])
{
    char *dir_name;
    dir_block_t db;
    int ino;
    int i;
    if (argc < 2) 
        return;
    else
        dir_name = argv[1];
    dir_block_read(&db, shell_path_inode.ino);

    for (i = 0; i < DENTRYS_PER_BLOCK; i++) {
        if (db.dentry[i].ino == -1)
            break;
        // if (bitmap_getbit(&inode_map, db.dentry[i].ino) == 0)
        //     break;
    }

    ino = inode_alloc();
    inode_create(ino, dir, dir_name, shell_path_inode.ino);
    db.dentry[i].ino = ino;
    db.dentry[i].mode = dir;
    strcpy(db.dentry[i].name, dir_name);

    dir_block_write(&db, shell_path_inode.ino);
}

void remove_directory(int argc, char *argv[])
{
    char *dir_name;
    int i;
    int ino;
    dir_block_t db;
    if (argc < 2)
        return;
    else
        dir_name = argv[1];
    
    dir_block_read(&db, shell_path_inode.ino);
    if (strcmp(dir_name, ".") == 0 || strcmp(dir_name, "..") == 0) {
        printf("error: you cannot remove this directory!\n");
        return;
    }

    for (i = 0; i < DENTRYS_PER_BLOCK; i++) {
        if (db.dentry[i].ino != -1 && db.dentry[i].mode == dir && strcmp(db.dentry[i].name, dir_name) == 0) {
            inode_delete(db.dentry[i].ino);
            db.dentry[i].ino = -1;
            // 这里没有递归删除路径内的内容，所以会造成资源浪费
            break;
        }
    }

    dir_block_write(&db, shell_path_inode.ino);
}

void remove_file(int argc, char *argv[])
{
    char *file_name;
    int i;
    int ino;
    dir_block_t db;
    if (argc < 2)
        return;
    else
        file_name = argv[1];
    
    dir_block_read(&db, shell_path_inode.ino);

    for (i = 0; i < DENTRYS_PER_BLOCK; i++) {
        if (db.dentry[i].ino != -1 && db.dentry[i].mode == file && strcmp(db.dentry[i].name, file_name) == 0) {
            inode_delete(db.dentry[i].ino);
            db.dentry[i].ino = -1;
            break;
        }
    }

    dir_block_write(&db, shell_path_inode.ino);
}

int decode_path(char *dir_name)
{
    // A/B/C -> A B C
    char *dir_args[16];
    int dir_num = 0;
    char buf[32];
    int i = 0;
    int j = 0;
    dir_block_t db;

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
    if (strlen(dir_name) == 0)
        return 1;
    
    for (i = 0; i < dir_num; i++) {
        dir_block_read(&db, cur_dir.ino);
        for (j = 0; j < DENTRYS_PER_BLOCK; j++) {
            if (db.dentry[j].ino == -1)
                continue;
            if (db.dentry[j].mode == file)
                continue;
            // if (bitmap_getbit(&inode_map, db.dentry[j].ino) == 0)
            //     continue;
            if (strcmp(db.dentry[j].name, dir_args[i]) == 0) {
                inode_read(&cur_dir, db.dentry[j].ino);
                break;
            }
        }
        if (j == DENTRYS_PER_BLOCK) {
            printf("error: no such directory\n");
            return -1;
        }
    }
    return 1;
}