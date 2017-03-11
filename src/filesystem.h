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

/* Directory entry masks */
#define VALID_DIR_ENTRY_MASK   0x80
#define DIR_ENTRY_IS_FILE_MASK 0x20
#define DIR_ENTRY_TRACK_MASK   0x1F
#define DIR_ENTRY_SECTOR_MASK  0x3F

/* Directory entry constants */
#define MAX_DIR_ENTRY_NAME_LENGTH 6
#define DIR_ENTRY_NAME_OFFSET     2


/*
 * Data structures
 */

typedef struct {
    char* name;
    unsigned int block;
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
 * Filesystem control functions
 */

Filesystem* mountFilesystem(char* filePath);
void unmountFilesystem(Filesystem* fs, char save);
void formatFilesystem(Filesystem* fs, char* volumeName);


#endif /* FILESYSTEM_H */
