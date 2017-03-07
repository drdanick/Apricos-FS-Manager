#include <stdlib.h>
#include "filesystem.h"
#include "diskio.h"


Filesystem* mountFilesystem(char* filePath) {
    Filesystem* fs = malloc(sizeof(Filesystem));

    fs->diskImagePath = filePath;
    fs->diskData = loadDataFromDiskFile(filePath);

    if(fs->diskData == NULL) {
        free(fs);
        return NULL;
    }

    fs->spaceBitmap = getSegmentData(fs->diskData, 0, 62);
    fs->volumeInformation = getSegmentData(fs->diskData, 0, 63);

    fs->currentPathUnit = -1;

    /* TODO: Attempt to mount the root of the disk as a folder */

    return fs;
}

void unmountFilesystem(Filesystem* fs, char save) {
    if(save)
        saveDiskFile(fs->diskImagePath, fs->diskData);
    free(fs);
}
