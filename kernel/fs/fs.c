#include "fs.h"
#include "time.h"
#include "string.h"

file_desc_t fd[FD_NUM];
uint32_t fd_index = 0;

// fs buffers
superblock_t superblock;
inode_map_t inode_map;

// fs buffers ended

void dir_init(inode_entry_t *inode);
void file_init(inode_entry_t *inode);

static void superblock_init()
{
    superblock.magic = FS_MAGIC;
    superblock.root_dir_ino = ROOT_DIR_INO;
    superblock.fs_size = 0x20000000;
    superblock.inode_map_base = FS_INODE_MAP_BASE;
    superblock.inode_map_size = FS_INODE_MAP_SIZE;
    superblock.inode_table_base = FS_INODE_TABLE_BASE;
    superblock.inode_table_size = FS_INODE_TABLE_SIZE;
}

static void inode_map_init()
{
    int i;
    for (i = 0; i < 0x400; i++) {
        inode_map.valid_inode_number[i] = 0;
    }
}

void do_makefs(void)
{
    superblock_read();
    if (superblock.magic == FS_MAGIC) {
        printf("Here is already a file system!\n");
        inode_map_read();
        mount();
        return;
    }

    superblock_init();
    inode_map_init();
    if (inode_alloc() != 0) {
        printf("inode map error\n");
        return;
    }
    inode_create(ROOT_DIR_INO, dir, "~", ROOT_DIR_INO);

    superblock_write();
    inode_map_write();
    mount();
}

void deletefs()
{
    superblock_read();
    if (superblock.magic != FS_MAGIC) {
        printf("Here is no file system here!\n");
        return;
    }
    superblock.magic = 0x00;
    superblock_write();
}

void statfs()
{
    superblock_read();
    printf("FS information:\nMagic: %x\nFS Size: %x\nInode map base/size: %x %x\nInode table base/size: %x %x\n", 
        superblock.magic, superblock.fs_size, 
        superblock.inode_map_base, superblock.inode_map_size, superblock.inode_table_base, superblock.inode_table_size);
}

void inode_create(uint32_t ino, enum inode_mode mode, char *name, uint32_t father_ino)
{
    inode_entry_t inode;
    inode.father_ino = father_ino;
    inode.ino = ino;
    inode.mode = mode;
    strcpy(inode.name, name);
    inode.ptr = FS_DATA_BASE + ino * FS_BLOCK_SIZE;
    inode.size = FS_BLOCK_SIZE;
    inode.timestamp = get_timer();
    if (inode.mode == dir) {
        dir_init(&inode);
    } else {
        file_init(&inode);
    }
    inode_write(&inode, ino);
}

void dir_init(inode_entry_t *inode)
{
    dir_block_t db;
    int i = 0;
    for (i = 0; i < DENTRYS_PER_BLOCK; i++) {
        db.dentry[i].ino = -1;
    }

    // init .
    db.dentry[0].ino = inode->ino;
    db.dentry[0].mode = dir;
    strcpy(db.dentry[0].name, ".");

    // init ..
    db.dentry[1].ino = inode->father_ino;
    db.dentry[1].mode = dir;
    strcpy(db.dentry[1].name, "..");

    dir_block_write(&db, inode->ino);
}

void file_init(inode_entry_t *inode)
{
    file_block_t fb;
    fb.file_content[0] = 0x00;
    file_block_write(&fb, inode->ino);
}

void inode_delete(uint32_t ino)
{
    bitmap_modify(&inode_map, 0, ino);
    inode_map_write();
}