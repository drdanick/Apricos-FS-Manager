#include <stdlib.h>
#include "allocator.h"
#include "diskio.h"

long long findFreeMemoryBlocks(Filesystem* fs, int count, unsigned int* outBlocks) {
    long long searchStart;
    int freeBlocks;

    /* Find the first free block */
    searchStart = findNextFreeBlock(fs, 0);

    if(searchStart == -1) {
        return -1;
    }

    do {
        int i = count;
        freeBlocks = 0;

        for( ; i > 0; i--) {
            printf("testing block %lld\n", searchStart);
            if(isBlockFree(fs, searchStart)) {
                printf("found free block: %lld\n", searchStart);
                if(++freeBlocks >= count) {
                    break;
                } else {
                    searchStart++;
                }
            } else {
                break;
            }
        }

        /* if freeBlocks is less than count, find the next free block and repeat */
        if(freeBlocks < count) {
            searchStart = findNextFreeBlock(fs, searchStart);
        } else { /* ...otherwise, we're done */
            break;
        }
    } while(searchStart < SECTORS);

    if(freeBlocks < count)
        return -1;

    /* Fill in the list of blocks backwards (so we don't have do declare new counters) */
    outBlocks += count - 1;
    for(; count > 0; count--) {
        *outBlocks = searchStart--;
        outBlocks--;
    }

    return searchStart;
}

long long findNextFreeBlock(Filesystem* fs, unsigned int start) {
    for( ; start < SECTORS; start++) {
        if(isBlockFree(fs, start))
            return start;
    }

    return -1;
}

char isSectorFree(Filesystem* fs, char trackNum, char sectorNum) {
    unsigned int blockNum = trackNum * SECTORS_PER_TRACK + sectorNum;

    return isBlockFree(fs, blockNum);
}

char isBlockFree(Filesystem* fs, long long blockNum) {
    long bitmapByteNumber = blockNum / 8;
    long bitmapByteOffset = blockNum % 8;

    return !((fs->spaceBitmap[bitmapByteNumber] >> bitmapByteOffset) & 0x01);
}
