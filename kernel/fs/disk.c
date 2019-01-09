#include "fs.h"

#define check_ino(ino) do { \ 
    if (ino < 0 || ino >= FS_INODE_TABLE_SIZE * 0x8) \
    return; \
} while(0)

#define sdmodify(sector_number, sector_buf, op) do { \
    sector_read(sector_buf, sector_number); \ 
    op; \ 
    sector_write(sector_buf, sector_number); \
} while (0)

void do_sdread(void *dest, int offset, int size) 
{
    sdread((char *)dest, offset + FS_BASE * SECTOR_SIZE, size);
}

void do_sdwrite(void *src, int offset, int size)
{
    sdwrite((char *)src, offset + FS_BASE * SECTOR_SIZE, size);
}

void sector_write(void *src, int sector)
{
    do_sdwrite(src, sector * SECTOR_SIZE, SECTOR_SIZE);
}

void sector_read(void *dest, int sector)
{
    do_sdread(dest, sector * SECTOR_SIZE, SECTOR_SIZE);
}

void inode_write(inode_entry_t *src, int ino)
{
    check_ino(ino);
    int sector = ino / 8 + FS_INODE_TABLE_BASE;
    int offset = ino % 8;
    inode_entry_t ie[8];
    sector_read(ie, sector);
    ie[offset] = *src;
    sector_write(ie, sector);
}

void inode_read(inode_entry_t *dest, int ino)
{
    check_ino(ino);
    do_sdread(dest, FS_INODE_TABLE_BASE * SECTOR_SIZE + ino * FS_INODE_SIZE, FS_INODE_SIZE);
}

void inode_map_read()
{
    int i;
    for (i = 0; i < FS_INODE_MAP_SIZE; i++) {
        sector_read(&inode_map.valid_inode_number[0x80 * i], FS_INODE_MAP_BASE + i);
    }
}

void inode_map_write()
{
    int i;
    for (i = 0; i < FS_INODE_MAP_SIZE; i++) {
        sector_write(&inode_map.valid_inode_number[0x80 * i], FS_INODE_MAP_BASE + i);
    }
}

int inode_alloc()
{
    int i = bitmap_search(&inode_map, 0, FS_INODE_TABLE_SIZE * 0x08);
    bitmap_modify(&inode_map, 1, i);
    inode_map_write();
    return i;
}

void inode_free(uint32_t ino)
{
    check_ino(ino);
    bitmap_modify(&inode_map, 0, ino);
    inode_map_write();
}

void superblock_read()
{
    sector_read(&superblock, FS_SUPERBLOCK_BASE);
}

void superblock_write()
{
    sector_write(&superblock, FS_SUPERBLOCK_BASE);
}

void file_block_read(file_block_t *fb, int ino)
{
    int i;
    int sector = ino * FS_BLOCK_SIZE + FS_DATA_BASE;
    for (i = 0; i < FS_BLOCK_SIZE; i++) {
        sector_read(&fb->file_content[0x80 * i], sector + i);
    }
}

void file_block_write(file_block_t *fb, int ino)
{
    int i;
    int sector = ino * FS_BLOCK_SIZE + FS_DATA_BASE;
    for (i = 0; i < FS_BLOCK_SIZE; i++) {
        sector_write(&fb->file_content[0x80 * i], sector + i);
    }
}

void dir_block_read(dir_block_t *db, int ino)
{
    int i;
    int sector = ino * FS_BLOCK_SIZE + FS_DATA_BASE;
    for (i = 0; i < FS_BLOCK_SIZE; i++) {
        sector_read(&db->dentry[0x10 * i], sector + i);
    }
}

void dir_block_write(dir_block_t *db, int ino)
{
    int i;
    int sector = ino * FS_BLOCK_SIZE + FS_DATA_BASE;
    for (i = 0; i < FS_BLOCK_SIZE; i++) {
        sector_write(&db->dentry[0x10 * i], sector + i);
    }
}