#include <string.h>
#include "fsfile.h"
#include "fsdirectory.h"
#include "allocator.h"

long long createFile(Filesystem* fs, FsDirectory* parentDir, char* name) {
    long long fileMetadataBlock = findNextFreeBlock(fs, 0);

    if(fileMetadataBlock != -1 && createFileAtBlock(fs, parentDir, fileMetadataBlock, name)) {
        return fileMetadataBlock;
    }

    return -1;
}

int createFileAtBlock(Filesystem* fs, FsDirectory* parentDir, unsigned int blockNum, char* name) {
    char* fileMetadata;
    int nextFreeDirEntry = findNextFreeDirEntry(parentDir);

    if(nextFreeDirEntry == -1 || !allocateBlock(fs, blockNum)) {
        return 0;
    }

    /* Obtain and clear the file metadata sector */
    fileMetadata = getBlockData(fs->diskData, blockNum);
    memset(fileMetadata, '\0', SECTOR_SIZE);


    /* Add the block to the parent directory */
    if(!addBlockEntrytoDirectory(parentDir, blockNum, name, 1)) {
        /* since this failed, free the previously allocated block */
        freeBlock(fs, blockNum);
        return 0;
    }

    return 1;
}

/* TODO: Add function to clear all blocks allocated for a file */
