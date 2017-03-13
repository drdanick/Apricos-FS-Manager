#include <stdlib.h>
#include <string.h>
#include "filesystem.h"
#include "apricosfsman.h"
#include "diskio.h"
#include "allocator.h"
#include "fsdirectory.h"

#define BOOT_SIGNATURE_SIZE 2
static const char bootSignature[] = {0xAA, 0x55};

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

    fs->currentPathUnit = 0;

    /* open and push the root directory */
    pushDirectoryToStack(fs, openBlockAsDirectory(fs, TRACK_AND_SECTOR_TO_BLOCK(ROOT_FOLDER_TRACK, ROOT_FOLDER_SECTOR), ""));

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

void clearDirectoryStack(Filesystem* fs) {
    fs->currentPathUnit = 0;
}

void formatFilesystem(Filesystem* fs, char* volumeName) {
    static unsigned int allocatedBlocks[SECTORS_PER_TRACK];

    /* clear space bitmap */
    memset(fs->spaceBitmap, 0, SECTOR_SIZE);

    /* clear volume information */
    memset(fs->rawVolumeInfo, 0, SECTOR_SIZE);

    /* set volume name */
    memcpy(fs->volumeInfo->volumeName, volumeName, MIN(strlen(volumeName), VOLUME_NAME_LENGTH));

    /* make volume bootable */
    memcpy(&(fs->rawVolumeInfo[SECTOR_SIZE - BOOT_SIGNATURE_SIZE]), bootSignature, BOOT_SIGNATURE_SIZE);

    /* Allocate the first track of data in the space bitmap */
    autoAllocateBlocks(fs, SECTORS_PER_TRACK, allocatedBlocks);

    /* create the root directory */
    createDirectory(fs);
}

void printPathString(Filesystem* fs) {
    static char nameBuff[MAX_DIR_ENTRY_NAME_LENGTH + 1];
    int i;

    /* In case the null terminator is missing, we have to do this */
    memset(nameBuff, '\0', MAX_DIR_ENTRY_NAME_LENGTH + 1);

    for(i = 0 ; i < fs->currentPathUnit; i++) {
        FsDirectory dir = fs->pathStack[i];
        memcpy(nameBuff, dir.name, strlen(dir.name));

        printf("/%s", nameBuff);
    }
}
