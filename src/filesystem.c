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
    /* clear space bitmap */
    memset(fs->spaceBitmap, 0, SECTOR_SIZE);

    /* clear volume information */
    memset(fs->rawVolumeInfo, 0, SECTOR_SIZE);

    /* set volume name */
    memcpy(fs->volumeInfo->volumeName, volumeName, MIN(strlen(volumeName), VOLUME_NAME_LENGTH));

    /* make volume bootable */
    memcpy(&(fs->rawVolumeInfo[SECTOR_SIZE - BOOT_SIGNATURE_SIZE]), bootSignature, BOOT_SIGNATURE_SIZE);

    /* create the root directory */
    /* TODO */

}
