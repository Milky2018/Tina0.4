#include "fs.h"

int bitmap_search(uint32_t *map, int target, int bound)
{
    int i = 0;
    int j = 0;
    uint32_t flat = 0;

    if (target == 1)
        flat = 0x00;
    else if (target == 0)
        flat = 0xffffffff;
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

int bitmap_getbit(uint32_t *map, int offset)
{
    int i = offset / 32;
    int j = offset % 32;
    return (map[i] >> j) & 0x0001;
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