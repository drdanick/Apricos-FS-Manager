#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdio.h>
#include "diskio.h"

/*
 * Constants
 */

#define MAX_PATH_DEPTH 64

/* disk layout constants */
#define VOLUME_INFORMATION_SEGMENT 62
#define SPACE_BITMAP_SEGMENT 63

/*
 * Data structures
 */

typedef struct {
    char* name;
    unsigned int length;
} FsPathUnit;

typedef struct {
    char volumeName[8];
} VolumeInfo;

typedef struct {
    char* diskImagePath;
    char* spaceBitmap;
    VolumeInfo* volumeInfo;
    char* diskData;
    FsPathUnit pathStack[MAX_PATH_DEPTH];
    int currentPathUnit;
} Filesystem;


/*
 * Functions
 */

Filesystem* mountFilesystem(char* filePath);
void unmountFilesystem(Filesystem* fs, char save);

#endif /* FILESYSTEM_H */
