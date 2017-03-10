#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "filesystem.h"
#include "diskio.h"

#define BLOCK_TO_TRACK(block) (block / SECTORS_PER_TRACK)
#define BLOCK_TO_SECTOR(block) (block % SECTORS_PER_TRACK)
#define TRACK_AND_SECTOR_TO_BLOCK(track, sector) (track * SECTORS_PER_TRACK + sector)

long long autoAllocateBlock(Filesystem* fs);
int autoAllocateBlocks(Filesystem* fs, unsigned int blockCount, unsigned int* outBlocks);
int autoAllocateContiguousBlocks(Filesystem* fs, unsigned int blockCount, unsigned int* outBlocks);
int allocateBlocks(Filesystem* fs, unsigned int* blockNums, unsigned int count);
int allocateSector(Filesystem* fs, char trackNum, char sectorNum);
int allocateBlock(Filesystem* fs, unsigned int blockNum);
int freeBlocks(Filesystem* fs, unsigned int* blockNums, unsigned int count);
int freeSector(Filesystem* fs, char trackNum, char sectorNum);
int freeBlock(Filesystem* fs, unsigned int blockNum);
long long findFreeMemoryBlocks(Filesystem* fs, int count, unsigned int* outBlocks);
long long findNextFreeBlock(Filesystem* fs, unsigned int start);
char isSectorFree(Filesystem* fs, char trackNum, char sectorNum);
char isBlockFree(Filesystem* fs, long long blockNum);

#endif /* ALLOCATOR_H */
