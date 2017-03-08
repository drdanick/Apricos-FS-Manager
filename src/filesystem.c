#include <stdlib.h>
#include <string.h>
#include "filesystem.h"
#include "diskio.h"

#define MIN(a, b) ((a < b) ? a : b)


Filesystem* mountFilesystem(char* filePath) {
    Filesystem* fs = malloc(sizeof(Filesystem));

    fs->diskImagePath = malloc(sizeof(char) * strlen(filePath));
    strcpy(fs->diskImagePath, filePath);
    fs->diskData = loadDataFromDiskFile(filePath);

    if(fs->diskData == NULL) {
        free(fs);
        return NULL;
    }

    fs->spaceBitmap = getSegmentData(fs->diskData, 0, 62);
    fs->volumeInfo = (VolumeInfo*)getSegmentData(fs->diskData, 0, 63);

    fs->currentPathUnit = -1;

    /* TODO: Attempt to mount the root of the disk as a folder */

    return fs;
}

void unmountFilesystem(Filesystem* fs, char save) {
    if(save)
        saveDiskFile(fs->diskImagePath, fs->diskData);
    free(fs->diskImagePath);
    free(fs);
}

void formatFilesystem(Filesystem* fs, char* volumeName) {
    /* clear space bitmap */
    memset(fs->spaceBitmap, 0, SECTOR_SIZE);

    /* set volume name */
    memcpy(fs->volumeInfo->volumeName, volumeName, MIN(strlen(volumeName), VOLUME_NAME_LENGTH));

    /* create the root directory */
    /* TODO */

}
