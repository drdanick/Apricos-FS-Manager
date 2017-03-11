#include <string.h>
#include "fsdirectory.h"
#include "apricosfsman.h"
#include "diskio.h"
#include "allocator.h"
#include "filesystem.h"

long long createDirectory(Filesystem* fs, char* name) {
    /* Allocate the block */
    long long directoryBlock = findNextFreeBlock(fs, 0);

    if(directoryBlock != -1 && createDirectoryAtBlock(fs, name, directoryBlock)) {
        return directoryBlock;
    }

    return -1;
}

int createDirectoryAtBlock(Filesystem* fs, char* name, unsigned int blockNum) {
    char* dirMetadataSector;
    if(!allocateBlock(fs, blockNum))
        return 0;

    dirMetadataSector = getBlockData(fs->diskData, blockNum);
    memset(dirMetadataSector, '\0', SECTOR_SIZE);

    /* copy the dir name into the dir metadata segment, starting at offset 2 */
    memcpy(&dirMetadataSector[DIR_ENTRY_NAME_OFFSET], name, MIN(MAX_DIR_ENTRY_NAME_LENGTH, strlen(name)));

    return 1;
}
