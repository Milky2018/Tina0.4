#include "fs.h"
#include "time.h"
#include "string.h"

file_desc_t fd[FD_NUM];
uint32_t fd_index = 0;

// fs buffers
superblock_t superblock;
sector_map_t sector_map;
inode_map_t inode_map;

// fs buffers ended

void do_makefs(void)
{
    int i;
    int root;
    do_sdread(&superblock, FS_SUPERBLOCK_BASE * SECTOR_SIZE, sizeof(superblock));
    // printf("superblock.root_dir_ino: %x\n", superblock.root_dir_ino);
    if (superblock.magic == FS_MAGIC) {
        printf("Here is already a file system!\n");
        mount();
        do_sdread(&sector_map, FS_SECTOR_MAP_BASE * SECTOR_SIZE, FS_SECTOR_MAP_SIZE * SECTOR_SIZE);
        do_sdread(&inode_map, FS_INODE_MAP_BASE * SECTOR_SIZE, FS_INODE_MAP_SIZE * SECTOR_SIZE);
        return;
    } else {
        printf("Please wait ...\n");
    }

// init superblock
    superblock.magic = FS_MAGIC;
    superblock.fs_size = FS_SIZE;
    superblock.inode_map_base = FS_INODE_MAP_BASE;
    superblock.inode_map_size = FS_INODE_MAP_SIZE;
    superblock.inode_table_base = FS_INODE_TABLE_BASE;
    superblock.inode_table_size = FS_INODE_TABLE_SIZE;
    superblock.sector_map_base = FS_SECTOR_MAP_BASE;
    superblock.sector_map_size = FS_SECTOR_MAP_SIZE;

// init sector map
    for (i = 0; i < FS_DATA_BASE / 32; i++) {
        sector_map.valid_sector_number[i] = 0xffff;
    }
    for (i = FS_DATA_BASE / 32; i < FS_SECTOR_NUM / 32; i++) {
        sector_map.valid_sector_number[i] = 0;
    }

// init inode map
    for (i = 0; i < FS_INODE_TABLE_SIZE / 32; i++) {
        inode_map.valid_inode_number[i] = 0;
    }
    root = inode_alloc();
    inode_init(root, path, "~", root);

// write all buffers
    superblock.root_dir_ino = root;
    do_sdwrite(&superblock, FS_SUPERBLOCK_BASE * SECTOR_SIZE, sizeof(superblock));
    do_sdwrite(&sector_map, FS_SECTOR_MAP_BASE * SECTOR_SIZE, FS_SECTOR_MAP_SIZE * SECTOR_SIZE);
    do_sdwrite(&inode_map, FS_INODE_MAP_BASE * SECTOR_SIZE, FS_INODE_MAP_SIZE * SECTOR_SIZE);

    printf("[mkfs] finished!\n");
    mount();
}

void deletefs()
{
    superblock.magic = 0x00;
    do_sdwrite(&superblock, FS_SUPERBLOCK_BASE * SECTOR_SIZE, sizeof(superblock));
    strcpy(shell_path_head, "");
}

void statfs()
{
    do_sdread(&superblock, FS_SUPERBLOCK_BASE * SECTOR_SIZE, sizeof(superblock));
    printf("FS information:\nMagic: %x\nFS Size: %x\nSector map base/size: %x %x\nInode map base/size: %x %x\nInode table base/size: %x %x\n", 
        superblock.magic, superblock.fs_size, superblock.sector_map_base, superblock.sector_map_size,
        superblock.inode_map_base, superblock.inode_map_size, superblock.inode_table_base, superblock.inode_table_size);
}

void inode_init(uint32_t ino, enum inode_mode mode, char *name, uint32_t father_ino)
{
    inode_entry_t inode;
    inode.ino = ino;
    inode.mode = mode;
    inode.ptr[0] = sector_alloc();
    inode.size = 0x200;
    inode.timestamp = get_timer();
    inode.father_ino = father_ino;
    strcpy(inode.name, name);
    if (inode.mode == path) {
        path_init(&inode);
    }
    inode_write(&inode, ino);
}

void print_sector_map(int start, int size)
{
    int k = size / 32 + 1;
    int i = start / 32;
    int j = 0;
    for (j = i; j < i + k; j++) {
        printf("%x ", sector_map.valid_sector_number[j]);
    }
    printf("\n");
}

int path_init(inode_entry_t *inode)
{
    dentry_sector ds;
    dentry_sector_init(&ds);

    // init .
    ds.den[0].ino = inode->ino;
    ds.den[0].mode = path;
    strcpy(ds.den[0].name, ".");

    // init ..
    ds.den[1].ino = inode->father_ino;
    ds.den[1].mode = path;
    strcpy(ds.den[1].name, "..");

    do_sdwrite(&ds, inode->ptr[0] * SECTOR_SIZE, SECTOR_SIZE);
}

void dentry_sector_init(dentry_sector *ds)
{
    int i;
    for (i = 0; i < 16; i++) {
        ds->den[i].ino = -1;
    }
}