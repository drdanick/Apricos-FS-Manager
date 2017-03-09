#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "filesystem.h"
#include "diskio.h"

#define BLOCK_TO_TRACK(block) (block / SECTORS_PER_TRACK)
#define BLOCK_TO_SECTOR(block) (block % SECTORS_PER_TRACK)

long long findFreeMemoryBlocks(Filesystem* fs, int count, unsigned int* outBlocks);
long long findNextFreeBlock(Filesystem* fs, unsigned int start);
char isSectorFree(Filesystem* fs, char trackNum, char sectorNum);
char isBlockFree(Filesystem* fs, long long blockNum);

#endif /* ALLOCATOR_H */
