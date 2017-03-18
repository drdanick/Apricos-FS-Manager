#include <string.h>
#include <stdio.h>
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
    ((FsFileMetadata*)fileMetadata)->fileSize = 0;

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

int getIndexOfFileBlock(FsFile* file, FsFileBlockPointer* blockPointer) {
    int i = 0;

    for( ; i < MAX_FILE_BLOCKS; i++) {
        if(&file->fileMetadata->filePointers[i] == blockPointer) {
            return i;
        }
    }

    return -1;
}

FsFileBlockPointer* getNextFileBlock(Filesystem* fs, FsFile* file, FsFileBlockPointer* currentBlockPointer) {
    int i = -1;

    if(currentBlockPointer) {
        i = getIndexOfFileBlock(file, currentBlockPointer);

        if(i == -1) {
            return NULL;
        }
    }

    for(i += 1; i < MAX_FILE_BLOCKS; i++) {
        FsFileBlockPointer* nextBlockPointer = &file->fileMetadata->filePointers[i];
        if(nextBlockPointer->track != 0 && !isSectorFree(fs, nextBlockPointer->track, nextBlockPointer->track)) {
            return nextBlockPointer;
        }
    }

    return NULL;
}

FsFileBlockPointer* getLastAllocatedFileBlock(Filesystem* fs, FsFile* file) {
    int i = 0;
    FsFileBlockPointer* block = NULL;
    do {
        FsFileBlockPointer* nextBlock = &file->fileMetadata->filePointers[i];

        if(isSectorFree(fs, nextBlock->track, nextBlock->sector)) {
            break;
        }

        block = nextBlock;
    } while(++i < MAX_FILE_BLOCKS);

    return block;
}

int allocateNewFileData(Filesystem* fs, FsFile* file, unsigned int numBytes, FsFileAllocatedSpacePointer* outBlocks, int maxBlocks) {
    unsigned int freeFileSpace = (MAX_FILE_BLOCKS * SECTOR_SIZE) - file->fileMetadata->fileSize;
    int residualExistingFreeSpace = freeFileSpace % SECTOR_SIZE;
    int newNumBytesRequired = numBytes - residualExistingFreeSpace;
    int newBlocksRequired = (int)((newNumBytesRequired - 1) / SECTOR_SIZE) + 1;
    int numChunks = 0;

    /* Clamp the new number of bytes required if we require less than what's currently available */
    newNumBytesRequired = newNumBytesRequired < 0 ? 0 : newNumBytesRequired;

    /* If the amount of space we need cannot be allocated for the current file, return -1 */
    if(numBytes > freeFileSpace) {
        return -1;
    }

    /* If the number of blocks required is less than maxBlocks, return -1 */
    if(newNumBytesRequired && newBlocksRequired > maxBlocks - 1) {
        return -1;
    }

    if(file->fileMetadata->fileSize && residualExistingFreeSpace) {
        int allocatedResidualData = MIN(numBytes, (unsigned int)residualExistingFreeSpace);
        FsFileBlockPointer* lastBlock = getLastAllocatedFileBlock(fs, file);
        char* ptr = getSectorData(fs->diskData, lastBlock->track, lastBlock->sector);
        ptr = &ptr[SECTOR_SIZE - residualExistingFreeSpace];

        outBlocks[numChunks].size = allocatedResidualData;
        outBlocks[numChunks].dataPointer = ptr;
        numChunks++;
    }

    for( ; numChunks < newBlocksRequired; numChunks++) {
        long long newBlock = allocateNewFileBlock(fs, file);
        outBlocks[numChunks].dataPointer = getBlockData(fs->diskData, newBlock);
        outBlocks[numChunks].size = newNumBytesRequired <= SECTOR_SIZE ? newNumBytesRequired : (char)(SECTOR_SIZE & 0x0FF);
        newNumBytesRequired -= SECTOR_SIZE;
    }

    file->fileMetadata->fileSize += numBytes;
    return numChunks;
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

    metadata->fileSize = 0;

    return !error;
}

long long appendDataFromFileStream(Filesystem* fs, FsFile* file, char* fileName) {
    static FsFileAllocatedSpacePointer newData[MAX_FILE_BLOCKS];
    static char diskBuffer[SECTOR_SIZE];
    long long totalBytesRead = 0;
    int currBytesRead;

    FILE* dataFile = fopen(fileName, "rb");
    if(!dataFile) {
        printf("cannot open %s\n",fileName);
        return -1;
    }



    while(currBytesRead = fread(diskBuffer, sizeof(char), sizeof(diskBuffer), dataFile), currBytesRead) {
        int newDataBlocks = allocateNewFileData(fs, file, currBytesRead, newData, MAX_FILE_BLOCKS);
        int bytesWritten = 0;
        int i = 0;
        totalBytesRead += currBytesRead;

        if(newDataBlocks < 1) {
            fclose(dataFile);
            return -1;
        }

        /* Copy data into the given blocks */
        for( ; i < newDataBlocks; i++) {
            memcpy(newData[i].dataPointer, &diskBuffer[bytesWritten], newData[i].size);
            bytesWritten += newData[i].size;
        }
    }

    fclose(dataFile);
    return totalBytesRead;
}

long long writeFileDataToFileStream(Filesystem* fs, FsFile* file, char* fileName) {
    unsigned int remainingData = file->fileMetadata->fileSize;
    FsFileBlockPointer* nextFileBlock = NULL;

    FILE* dataFile = fopen(fileName, "wb");
    if(!dataFile) {
        printf("Cannot open %s\n", fileName);
        return -1;
    }

    while(nextFileBlock = getNextFileBlock(fs, file, nextFileBlock), nextFileBlock) {
        int dataToCopy = remainingData;
        if(remainingData > SECTOR_SIZE) {
            dataToCopy = SECTOR_SIZE;
        }

        fwrite(getSectorData(fs->diskData, nextFileBlock->track, nextFileBlock->sector), sizeof(char), dataToCopy, dataFile);
        remainingData -= dataToCopy;
    }

    fclose(dataFile);
    return file->fileMetadata->fileSize - remainingData;
}
