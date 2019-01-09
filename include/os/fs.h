#ifndef __FILE_SYSTEM__
#define __FILE_SYSTEM__

#include "type.h"

#define FS_BASE 0x100000
#define SECTOR_SIZE 0x200

#define FS_MAGIC 0x0711
#define FS_DATA_BASE 0x00
#define FS_SUPERBLOCK_BASE 0xfdff7
#define FS_SUPERBLOCK_SIZE 0x01
#define FS_INODE_MAP_BASE 0xfdff8
#define FS_INODE_MAP_SIZE 0x08
#define FS_INODE_TABLE_BASE 0xfe000
#define FS_INODE_TABLE_SIZE 0x1000

#define FS_BLOCK_SIZE 0x1
#define FS_INODE_SIZE 0x40
#define FD_NUM 0x10

#define FS_DENTRY_SIZE 0x20
#define DENTRYS_PER_BLOCK (FS_BLOCK_SIZE * SECTOR_SIZE / FS_DENTRY_SIZE)
#define CONTENTS__PER_BLOCK (FS_BLOCK_SIZE * SECTOR_SIZE / 4)

#define ROOT_DIR_INO 0

typedef struct {
    uint32_t magic;
    uint32_t fs_size;
    uint32_t inode_map_base;
    uint32_t inode_map_size;
    uint32_t inode_table_base;
    uint32_t inode_table_size;
    uint32_t root_dir_ino;
} superblock_t;

enum inode_mode {file, dir};

typedef struct {
    char name[32];
    uint32_t ino;
    uint32_t size;
    enum inode_mode mode;
    uint32_t timestamp;
    uint32_t father_ino;
    uint32_t ptr;
    uint32_t unused[2];
} inode_entry_t;

typedef struct {
    uint32_t ino;
    uint32_t pos;
} file_desc_t;

typedef struct {
    uint32_t valid_inode_number[0x400];
} inode_map_t;

typedef struct {
    uint32_t ino;
    enum inode_mode mode;
    char name[24];
} dentry_t;

typedef struct {
    uint32_t file_content[CONTENTS__PER_BLOCK];
} file_block_t;

typedef struct {
    dentry_t dentry[DENTRYS_PER_BLOCK];
} dir_block_t;

extern superblock_t superblock;
extern inode_map_t inode_map;
extern inode_entry_t shell_path_inode;
extern char shell_path_head[128];
extern file_desc_t fd[FD_NUM];
extern uint32_t fd_index;

int bitmap_search(uint32_t *map, int target, int bound);
int bitmap_modify(uint32_t *map, int target, int offset);
int bitmap_getbit(uint32_t *map, int offset);
void inode_free(uint32_t ino);
int inode_alloc();
void inode_write(inode_entry_t *src, int ino);
void inode_read(inode_entry_t *dest, int ino);
void inode_map_read();
void inode_map_write();
void superblock_read();
void superblock_write();
void file_block_read(file_block_t *fb, int ino);
void file_block_write(file_block_t *fb, int ino);
void dir_block_read(dir_block_t *db, int ino);
void dir_block_write(dir_block_t *db, int ino);
void mount();
void deletefs();
void statfs();
void do_makefs(void);
void inode_create(uint32_t ino, enum inode_mode mode, char *name, uint32_t father_ino);
void inode_delete(uint32_t ino);
void change_directory(int argc, char *argv[]);
void list_directory(int argc, char *argv[]);
void make_directory(int argc, char *argv[]);
void remove_file(int argc, char *argv[]);
void remove_directory(int argc, char *argv[]);
void touch_file(int argc, char *argv[]);
void cat_file(int argc, char *argv[]);
void sys_fseek(int index, int pos);
void sys_fclose(int index);
int sys_fwrite(int index, char *buf, int size);
int sys_fread(int index, char *buf, int size);
int sys_fopen(char *name, int access);

#endif