#ifndef FILESYSTEM_H
#define FILESYSTEM_H

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
    char* diskImagePath;
    char* diskData;
    char* spaceBitmap;
    char* volumeInformation;
    FsPathUnit* pathStack;
} Filesystem;


/*
 * Functions
 */

void init(char* filePath);

#endif /* FILESYSTEM_H */
