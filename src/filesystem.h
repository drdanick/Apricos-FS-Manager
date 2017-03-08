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

/* Volume info constants */
#define VOLUME_NAME_LENGTH 8

/*
 * Data structures
 */

typedef struct {
    char* name;
    unsigned int length;
} FsPathUnit;

typedef struct {
    char volumeName[VOLUME_NAME_LENGTH];
} VolumeInfo;

typedef struct {
    char* diskImagePath;
    char* spaceBitmap;
    VolumeInfo* volumeInfo;
    char* rawVolumeInfo;
    char* diskData;
    FsPathUnit pathStack[MAX_PATH_DEPTH];
    int currentPathUnit;
} Filesystem;


/*
 * Functions
 */

Filesystem* mountFilesystem(char* filePath);
void unmountFilesystem(Filesystem* fs, char save);
void formatFilesystem(Filesystem* fs, char* volumeName);

#endif /* FILESYSTEM_H */
