#include <stdlib.h>
#include <string.h>
#include "fsdirectory.h"
#include "apricosfsman.h"
#include "diskio.h"
#include "allocator.h"
#include "filesystem.h"

long long createDirectory(Filesystem* fs) {
    /* Allocate the block */
    long long directoryBlock = findNextFreeBlock(fs, 0);

    if(directoryBlock != -1 && createDirectoryAtBlock(fs, directoryBlock)) {
        return directoryBlock;
    }

    return -1;
}

int createDirectoryAtBlock(Filesystem* fs, unsigned int blockNum) {
    char* dirMetadataSector;
    if(!allocateBlock(fs, blockNum))
        return 0;

    dirMetadataSector = getBlockData(fs->diskData, blockNum);
    memset(dirMetadataSector, '\0', SECTOR_SIZE);

    return 1;
}

int pushDirectoryToStack(Filesystem* fs, FsDirectory dir) {
    if(fs->currentPathUnit < MAX_PATH_DEPTH) {
        fs->pathStack[fs->currentPathUnit] = dir;
        fs->currentPathUnit++;

        return 1;
    }

    return -1;
}

FsDirectory popDirectoryFromStack(Filesystem* fs) {
    FsDirectory directory;

    if(fs->currentPathUnit <= 0) {
        directory.name = NULL;
        directory.rawData = NULL;
        directory.dirEntries = NULL;
    } else {
        directory = fs->pathStack[fs->currentPathUnit--];
    }

    return directory;
}

FsDirectory openBlockAsDirectory(Filesystem* fs, unsigned int blockNum, char* dirName) {
    FsDirectory directory;
    directory.name = NULL;

    if(isBlockFree(fs, blockNum)) {
        directory.name = NULL;
        directory.rawData = NULL;
        directory.dirEntries = NULL;
    } else {
        char* dirMetadataSector = getBlockData(fs->diskData, blockNum);

        directory.name = dirName;
        directory.block = blockNum;
        directory.rawData = dirMetadataSector;
        directory.dirEntries = (FsDirectoryEntry*) dirMetadataSector;
    }

    return directory;
}
