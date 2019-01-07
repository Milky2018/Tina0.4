#ifndef __FILE_SYSTEM__
#define __FILE_SYSTEM__

#include "type.h"

#define FS_SECTORADDR_BASE 0x100000

#define FS_SUPERBLOCK_BASE 0x00
#define FS_SUPERBLOCK_SIZE 0x01
#define SECTOR_SIZE 0x200
#define FS_SIZE 0x20000000
#define FS_SECTOR_MAP_BASE 0x01
#define FS_SECTOR_MAP_SIZE 0x100
#define FS_INODE_MAP_BASE 0x101
#define FS_INODE_MAP_SIZE 0x08
#define FS_MAGIC 0x0711
#define FS_INODE_TABLE_BASE 0x109
#define FS_INODE_TABLE_SIZE 0x1000
#define FS_DATA_BASE 0x2000
#define FS_INODE_ENTRY_SIZE 0x200
#define FD_NUM 0x10
#define FS_SECTOR_NUM (FS_SIZE / SECTOR_SIZE) // 0x100000

// valid initially sectors are:
// superblock: 0x0
// sector map: 0x1 ~ 0x100
// inode map: 0x101 ~ 0x108
// inodes sector: 0x109 ~ 0x508

typedef struct {
    uint32_t magic;
    uint32_t fs_size;
    uint32_t sector_map_base;
    uint32_t sector_map_size;
    uint32_t inode_map_base;
    uint32_t inode_map_size;
    uint32_t inode_table_base;
    uint32_t inode_table_size;
    uint32_t root_dir_ino;
} superblock_t;

enum inode_mode {file, path};

typedef struct {
    char name[32];
    uint32_t ino;
    uint32_t size;
    enum inode_mode mode;
    uint32_t timestamp;
    uint32_t father_ino;
    uint32_t ptr[19];
} inode_entry_t;

typedef struct {
    uint32_t ino;
    uint32_t pos;
} file_desc_t;

typedef struct {
    uint32_t valid_sector_number[0x8000];
} sector_map_t;

typedef struct {
    uint32_t valid_inode_number[0x400];
} inode_map_t;

typedef struct {
    uint32_t ino;
    enum inode_mode mode;
    char name[24];
} dentry;

typedef struct {
    dentry den[16];
} dentry_sector;

extern superblock_t superblock;
extern sector_map_t sector_map;
extern inode_map_t inode_map;
extern inode_entry_t shell_path_inode;
extern char shell_path_head[128];

void do_sdread(void *dest, int offset, int size);
void do_sdwrite(void *src, int offset, int size);
void sector_write(char *src, int sector);
void inode_write(inode_entry_t *src, int ino);
void inode_read(inode_entry_t *dest, int ino);
int bitmap_search(uint32_t *map, int target, int bound);
int bitmap_modify(uint32_t *map, int target, int offset);
void do_makefs(void);
void statfs();
int sector_alloc();
void sector_free(uint32_t sector);
int inode_alloc();
void inode_free(uint32_t ino);
void inode_init(uint32_t ino, enum inode_mode mode, char *name, uint32_t father_ino);
void print_sector_map(int start, int size);
void deletefs();

int path_init(inode_entry_t *inode);

void make_directory(char *dir_name);
void change_directory(char *dir_name);
void list_directory(char *dir_name);
void mount();
void remove(char *dir_name);

void print_inode_entry(inode_entry_t *);

#endif