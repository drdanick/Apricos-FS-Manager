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

int isDirEntryFree(FsDirectoryEntry* entry) {
    return !(entry->markerAndTrackNum & VALID_DIR_ENTRY_MASK);
}

int findNextFreeDirEntry(FsDirectory* dir) {
    int i = 0;
    for( ; i < MAX_DIR_ENTRIES; i++) {
        if(isDirEntryFree(&dir->dirEntries[i])) {
            return i;
        }
    }

    return -1;
}

FsDirectoryEntry* findDirEntryByName(FsDirectory* dir, char* name) {
    int i = 0;
    int namelen = MIN(MAX_DIR_ENTRY_NAME_LENGTH, strlen(name));
    FsDirectoryEntry* entries = dir->dirEntries;

    for( ; i < MAX_DIR_ENTRIES; i++) {
        FsDirectoryEntry* entry = &entries[i];
        if(!isDirEntryFree(entry) && strncmp(entry->name, name, namelen) == 0)
            return entry;
    }

    return NULL;
}

void printDirectoryListing(FsDirectory* dir, char* prefix) {
    char nameBuff[MAX_DIR_ENTRY_NAME_LENGTH + 1]; /* Used to convert dir names to c style strings with null terminator */
    FsDirectoryEntry* entry;
    int i = 0;

    if(!prefix)
        prefix = "";

    memset(nameBuff, '\0', MAX_DIR_ENTRY_NAME_LENGTH + 1);

    while(entry = &dir->dirEntries[i++], i < MAX_DIR_ENTRIES && !isDirEntryFree(entry)) {
        memcpy(nameBuff, entry->name, sizeof(nameBuff));
        printf("%s%s\n", prefix, nameBuff);
    }
}

int removeDirEntry(FsDirectoryEntry* entry) {
    if(isDirEntryFree(entry)) {
        return 0;
    }

    /* clear the first byte */
    entry->markerAndTrackNum = 0x00;

    /* clear the name */
    memset(entry->name, '\0', MAX_DIR_ENTRY_NAME_LENGTH);

    return 1;
}

int removeDirEntryByName(FsDirectory* dir, char* name) {
    FsDirectoryEntry* entry = findDirEntryByName(dir, name);

    if(entry == NULL) {
        return 0;
    }

    return removeDirEntry(entry);
}

int allocateAndAddDirectoryEntryToDirectory(Filesystem* fs, FsDirectory* parentDir, char* childName) {
    long long blockNum = autoAllocateBlock(fs);

    if(blockNum < 0)
        return 0;

    return addDirectoryBlockEntrytoDirectory(parentDir, blockNum, childName);
}

int addDirectoryBlockEntrytoDirectory(FsDirectory* parentDir, unsigned int childBlock, char* childName) {
    int childTrack = BLOCK_TO_TRACK(childBlock);
    int childSector = BLOCK_TO_SECTOR(childBlock);

    return addDirectoryEntrytoDirectory(parentDir, childTrack, childSector, childName);
}

int addDirectoryEntrytoDirectory(FsDirectory* parentDir, int childtrack, int childSector, char* childName) {
    FsDirectoryEntry* entry;
    int childIndex = findNextFreeDirEntry(parentDir);

    entry = findDirEntryByName(parentDir, childName);

    if(entry || childIndex == -1) {
        return -1;
    }

    entry = &parentDir->dirEntries[childIndex];

    entry->markerAndTrackNum =  VALID_DIR_ENTRY_MASK | DIR_ENTRY_TYPE_MASK | (childtrack & DIR_ENTRY_TRACK_MASK);
    entry->sectorNum = childSector & DIR_ENTRY_SECTOR_MASK;
    memcpy(entry->name, childName, MIN(MAX_DIR_ENTRY_NAME_LENGTH, strlen(childName)));

    return 1;
}
