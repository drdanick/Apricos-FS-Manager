#include <stdlib.h>
#include <string.h>
#include "filesystem.h"
#include "apricosfsman.h"
#include "diskio.h"
#include "allocator.h"
#include "fsdirectory.h"

const char bootSignature[] = {0xAA, 0x55};
const char diskSignature[] = {0xC0, 0x30, 0x0C, 0x03};

Filesystem* mountFilesystem(char* filePath) {
    Filesystem* fs = malloc(sizeof(Filesystem));

    fs->diskImagePath = malloc(sizeof(char) * strlen(filePath) + 1);
    strcpy(fs->diskImagePath, filePath);
    fs->diskData = loadDataFromDiskFile(filePath);

    if(fs->diskData == NULL) {
        free(fs);
        return NULL;
    }

    fs->spaceBitmap = getSectorData(fs->diskData, 0, SPACE_BITMAP_SEGMENT);
    fs->volumeInfo = (VolumeInfo*)getSectorData(fs->diskData, 0, VOLUME_INFORMATION_SEGMENT);
    fs->rawVolumeInfo = getSectorData(fs->diskData, 0, VOLUME_INFORMATION_SEGMENT);

    resetDirectoryStack(fs);

    /* validate the disk signature */
    if(strncmp(fs->volumeInfo->diskSignature, diskSignature, DISK_SIGNATURE_SIZE) != 0) {
        unmountFilesystem(fs, 0);
        return NULL;
    }

    return fs;
}

void unmountFilesystem(Filesystem* fs, char save) {
    if(save)
        saveDiskFile(fs->diskImagePath, fs->diskData);
    clearDirectoryStack(fs);
    freeDiskData(fs->diskData);
    free(fs->diskImagePath);
    free(fs);
}

void resetDirectoryStack(Filesystem* fs) {
    FsDirectory rootDir;
    clearDirectoryStack(fs);

    /* open and push the root directory */
    if(openBlockAsDirectory(fs, TRACK_AND_SECTOR_TO_BLOCK(ROOT_FOLDER_TRACK, ROOT_FOLDER_SECTOR), "", &rootDir)) {
        pushDirectoryToStack(fs, rootDir);
    }
}

void clearDirectoryStack(Filesystem* fs) {
    fs->currentPathUnit = 0;
}

FsDirectory* getWorkingDirectory(Filesystem* fs) {
    if(!fs || fs->currentPathUnit == 0)
        return NULL;
    return &fs->pathStack[fs->currentPathUnit - 1];
}

void formatFilesystem(Filesystem* fs, char* volumeName) {
    static unsigned int allocatedBlocks[SECTORS_PER_TRACK];

    /* clear space bitmap */
    memset(fs->spaceBitmap, 0, SECTOR_SIZE);

    /* clear volume information */
    memset(fs->rawVolumeInfo, 0, SECTOR_SIZE);

    /* set disk signature */
    memcpy(fs->volumeInfo->diskSignature, diskSignature, DISK_SIGNATURE_SIZE);

    /* set volume name */
    memcpy(fs->volumeInfo->volumeName, volumeName, MIN(strlen(volumeName), VOLUME_NAME_LENGTH));

    /* make volume bootable */
    memcpy(&(fs->rawVolumeInfo[SECTOR_SIZE - BOOT_SIGNATURE_SIZE]), bootSignature, BOOT_SIGNATURE_SIZE);

    /* allocate the first track of data in the space bitmap */
    autoAllocateBlocks(fs, SECTORS_PER_TRACK, allocatedBlocks);

    /* create the root directory */
    createDirectory(fs);

    /* reset the directory stack to effectively mount this volume */
    resetDirectoryStack(fs);
}

void printPathString(Filesystem* fs) {
    static char nameBuff[MAX_DIR_ENTRY_NAME_LENGTH + 1];
    int i;

    /* In case the null terminator is missing, we have to do this */
    memset(nameBuff, '\0', MAX_DIR_ENTRY_NAME_LENGTH + 1);

    for(i = 0 ; i < fs->currentPathUnit; i++) {
        FsDirectory dir = fs->pathStack[i];
        memcpy(nameBuff, dir.name, strlen(dir.name));

        printf("%s/", nameBuff);
    }
}

FsDirectory* savePathStack(Filesystem* fs) {
    FsDirectory* dirStackCopy = malloc(sizeof(FsDirectory) * MAX_PATH_DEPTH);
    int i = 0;

    for( ; i < fs->currentPathUnit; i++) {
        dirStackCopy[i] = fs->pathStack[i];
    }

    return dirStackCopy;
}

void restorePathStack(Filesystem* fs, FsDirectory* srcStack, int srcStackNextFreeSlot) {
    int i = 0;

    for( ; i < srcStackNextFreeSlot; i++) {
        fs->pathStack[i] = srcStack[i];
    }
    fs->currentPathUnit = srcStackNextFreeSlot;

    free(srcStack);
}

void freePathStack(FsDirectory* pathStack) {
    free(pathStack);
}
