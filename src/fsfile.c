#include <string.h>
#include "fsfile.h"
#include "filesystem.h"
#include "fsdirectory.h"
#include "allocator.h"
#include "apricosfsman.h"

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

    /* Initialize the size field */
    ((FsFileMetadata*)fileMetadata)->fileSize = SECTOR_SIZE & FILE_METADATA_SIZE_MASK;

    return 1;
}

int openBlockAsFile(Filesystem* fs, unsigned int blockNum, char* fileName, FsFile* file) {
    if(!file) {
        return 0;
    }

    if(isBlockFree(fs, blockNum)) {
        return 0;
    } else {
        memset(file->name, '\0', sizeof(file->name));
        memcpy(file->name, fileName, MIN(MAX_DIR_ENTRY_NAME_LENGTH, strlen(fileName)));
        file->rawData = getBlockData(fs->diskData, blockNum);
        file->fileMetadata = (FsFileMetadata*)file->rawData;

        return 1;
    }
}

int getFsFileFromEntry(Filesystem* fs, FsDirectoryEntry* entry, FsFile* file) {
    static char nameBuffer[sizeof(file->name)];

    if(!file) {
        return 0;
    }

    if(!fs || !entry || !(entry->markerAndTrackNum & DIR_ENTRY_TYPE_MASK)) {
        return 0;
    } else {
        unsigned int track = entry->markerAndTrackNum & TRACK_ID_MASK;
        unsigned int sector = entry->sectorNum & SECTOR_ID_MASK;
        unsigned int block = TRACK_AND_SECTOR_TO_BLOCK(track, sector);

        /* Convert entry name into a C style string */
        memset(nameBuffer, '\0', sizeof(nameBuffer));
        memcpy(nameBuffer, entry->name, MAX_DIR_ENTRY_NAME_LENGTH);

        return openBlockAsFile(fs, block, nameBuffer, file);
    }
}

unsigned int calculateFileSize(Filesystem* fs, FsFile* file) {
    /* Account for the metadata sector */
    unsigned int size = SECTOR_SIZE;
    FsFileMetadata* metadata = file->fileMetadata;
    int i = 0;

    if(!file || !metadata) {
        return 0;
    }

    for( ; i < MAX_FILE_BLOCKS; i++) {
        FsFileBlockPointer* blockPointer = &metadata->filePointers[i];
        if(!blockPointer->track) {
            continue;
        }

        if(!isSectorFree(fs, blockPointer->track, blockPointer->sector)) {
            size += SECTOR_SIZE;
        }
    }

    return size;
}

long long allocateNewFileBlock(Filesystem* fs, FsFile* file) {
    FsFileMetadata* metadata = file->fileMetadata;
    int i = 0;

    for( ; i < MAX_FILE_BLOCKS; i++) {
        if(!metadata->filePointers[i].track) {
            long long newBlock = autoAllocateBlock(fs);
            unsigned int track = BLOCK_TO_TRACK(newBlock);
            unsigned int sector = BLOCK_TO_SECTOR(newBlock);

            if(newBlock == -1) {
                return -1;
            }

            metadata->filePointers[i].track = track;
            metadata->filePointers[i].sector = sector;

            metadata->fileSize += SECTOR_SIZE;

            return newBlock;
        }
    }

    return -1;
}

int deleteAllFileBlocks(Filesystem* fs, FsFile* file) {
    FsFileMetadata* metadata = file->fileMetadata;
    int i = 0;
    int error = 0;

    for( ; i < MAX_FILE_BLOCKS; i++) {
        FsFileBlockPointer* blockPointer = &metadata->filePointers[i];
        if(!blockPointer->track) {
            continue;
        }

        if(!freeSector(fs, blockPointer->track, blockPointer->sector)) {
            error = 1;
            continue;
        }

        blockPointer->track = 0;
        blockPointer->sector = 0;
    }

    metadata->fileSize = calculateFileSize(fs, file) & FILE_METADATA_SIZE_MASK;

    return !error;
}
