#include <stdlib.h>
#include "allocator.h"
#include "diskio.h"

long long autoAllocateBlock(Filesystem* fs) {
    long long freeBlock = findNextFreeBlock(fs, 0);

    if(freeBlock == -1)
        return -1;

    if(!allocateBlock(fs, freeBlock))
        return -1;

    return freeBlock;
}

int autoAllocateBlocks(Filesystem* fs, unsigned int blockCount, unsigned int* outBlocks) {
    long long searchStart = 0;
    unsigned int allocated = 0;

    for( ; blockCount > 0; blockCount--) {
        searchStart = findNextFreeBlock(fs, searchStart);

        if(searchStart != -1 && allocateBlock(fs, searchStart)) {
            outBlocks[allocated++] = searchStart;
        } else {
            freeBlocks(fs, outBlocks, allocated);
            return 0;
        }
    }
    
    return 1;
}

int autoAllocateContiguousBlocks(Filesystem* fs, unsigned int blockCount, unsigned int* outBlocks) {
    if(findFreeMemoryBlocks(fs, blockCount, outBlocks) == -1) {
        return 0;
    }

    return allocateBlocks(fs, outBlocks, blockCount);
}

int allocateBlocks(Filesystem* fs, unsigned int* blockNums, unsigned int count) {
    unsigned int i = 0;

    for( ; i < count; i++) {
        if(!allocateBlock(fs, blockNums[i])) {
            freeBlocks(fs, blockNums, i); /* Free previously allocated blocks */
            return 0;
        }
    }

    return 1;
}

int allocateSector(Filesystem* fs, char trackNum, char sectorNum) {
    return allocateBlock(fs, TRACK_AND_SECTOR_TO_BLOCK(trackNum, sectorNum));
}

int allocateBlock(Filesystem* fs, unsigned int blockNum) {
    long bitmapByteNumber = blockNum / 8;
    long bitmapByteOffset = blockNum % 8;

    if(blockNum < SECTORS && isBlockFree(fs, blockNum)) {
        fs->spaceBitmap[bitmapByteNumber] |= (0x01 << bitmapByteOffset);
        return 1;
    }

    return 0;
}

int freeBlocks(Filesystem* fs, unsigned int* blockNums, unsigned int count) {
    unsigned int i = 0;

    for( ; i < count; i++) {
        if(!freeBlock(fs, blockNums[i])) {
            allocateBlocks(fs, blockNums, i); /* Allocate previously freed blocks */
            return 0;
        }
    }

    return 1;
}

int freeSector(Filesystem* fs, char trackNum, char sectorNum) {
    return freeBlock(fs, TRACK_AND_SECTOR_TO_BLOCK(trackNum, sectorNum));
}

int freeBlock(Filesystem* fs, unsigned int blockNum) {
    long bitmapByteNumber = blockNum / 8;
    long bitmapByteOffset = blockNum % 8;

    if(blockNum < SECTORS) {
        fs->spaceBitmap[bitmapByteNumber] &= ~(0x01 << bitmapByteOffset);
        return 1;
    }

    return 0;
}


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
            if(isBlockFree(fs, searchStart)) {
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

    return searchStart + 1;
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
