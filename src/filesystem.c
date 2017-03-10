#include <stdlib.h>
#include <string.h>
#include "filesystem.h"
#include "diskio.h"
#include "allocator.h"

#define MIN(a, b) ((a < b) ? a : b)

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

    fs->currentPathUnit = -1;

    /* TODO: Attempt to mount the root of the disk as a folder */

    return fs;
}

void unmountFilesystem(Filesystem* fs, char save) {
    if(save)
        saveDiskFile(fs->diskImagePath, fs->diskData);
    freeDiskData(fs->diskData);
    free(fs->diskImagePath);
    free(fs);
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
    createDirectory(fs, "ROOT");

    /* find first free block of 8 sectors */
    {
        int i;
        unsigned int freeBlocks[8];
        findFreeMemoryBlocks(fs, 8, freeBlocks);
        printf("free blocks:\n");
        for(i = 0; i < 8; i++ ) {
            unsigned int block = freeBlocks[i];
            printf("block: %d, track: %d, sector: %d\n", block, BLOCK_TO_TRACK(block), BLOCK_TO_SECTOR(block));
        }
    }
}

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
    memcpy(&dirMetadataSector[2], name, MIN(MAX_DIR_ENTRY_NAME_LENGTH, strlen(name)));

    return 1;
}
