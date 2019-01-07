#include "fs.h"

void do_sdread(void *dest, int offset, int size) 
{
    sdread((char *)dest, offset + FS_SECTORADDR_BASE * SECTOR_SIZE, size);
}

void do_sdwrite(void *src, int offset, int size)
{
    // if (offset / 0x200 == 0x109)
    //     printf("109 occurred!\n");
    sdwrite((char *)src, offset + FS_SECTORADDR_BASE * SECTOR_SIZE, size);
}

void sector_write(char *src, int sector)
{
    do_sdwrite(src, sector * SECTOR_SIZE, SECTOR_SIZE);
}

void inode_write(inode_entry_t *src, int ino)
{
    do_sdwrite(src, FS_INODE_TABLE_BASE * SECTOR_SIZE + ino * FS_INODE_ENTRY_SIZE, FS_INODE_ENTRY_SIZE);
}

void inode_read(inode_entry_t *dest, int ino)
{
    do_sdread(dest, FS_INODE_TABLE_BASE * SECTOR_SIZE + ino * FS_INODE_ENTRY_SIZE, FS_INODE_ENTRY_SIZE);
}

int bitmap_search(uint32_t *map, int target, int bound)
{
    int i = 0;
    int j = 0;
    uint32_t flat = 0;

    if (target == 1)
        flat = 0x00;
    else if (target == 0)
        flat = 0xffff;
    else 
        return -1;
    
    for (i = 0; i < bound / 32; i++) {
        if (map[i] != flat) {
            for (j = 0; j < 32; j++) {
                if (((uint32_t)(0x01 << j) & map[i]) == (target << j)) {
                    return 32 * i + j;
                }
            }
        }
    }
    return -2;
}

int bitmap_modify(uint32_t *map, int target, int offset)
{
    int i, j;
    if (target != 0 && target != 1)
        return -1;

    i = offset / 32;
    j = offset % 32;
    map[i] = (map[i] & ~(0x0001 << j)) | (target << j);
    return 0;
}

int sector_alloc()
{
    int i = bitmap_search(&sector_map, 0, FS_SECTOR_NUM);
    bitmap_modify(&sector_map, 1, i);
    do_sdwrite(&sector_map.valid_sector_number[i / 32], FS_SECTOR_MAP_BASE * SECTOR_SIZE + (i / 32) * 4, 4);
    return i;
}

void sector_free(uint32_t sector)
{
    if (sector < 0 || sector >= FS_SECTOR_NUM)
        return;
    bitmap_modify(&sector_map, 0, sector);
    do_sdwrite(&sector_map.valid_sector_number[sector / 32], FS_SECTOR_MAP_BASE * SECTOR_SIZE + (sector / 32) * 4, 4);
}

int inode_alloc()
{
    int i = bitmap_search(&inode_map, 0, FS_INODE_TABLE_SIZE * 0x4);
    bitmap_modify(&inode_map, 1, i);
    do_sdwrite(&inode_map.valid_inode_number[i / 32], FS_INODE_MAP_BASE * SECTOR_SIZE + (i / 32) * 4, 4);
    return i;
}

void inode_free(uint32_t ino)
{
    if (ino < 0 || ino >= FS_INODE_TABLE_SIZE * 0x4)
        return;
    bitmap_modify(&inode_map, 0, ino);
    do_sdwrite(&inode_map.valid_inode_number[ino / 32], FS_INODE_MAP_BASE * SECTOR_SIZE + (ino / 32) * 4, 4);
}

void print_inode_entry(inode_entry_t *inode)
{
    printf("ino: %x    name: %s    ptr[0]: %x\n", inode->ino, inode->name, inode->ptr[0]);
}