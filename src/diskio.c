#include <stdlib.h>
#include <stdio.h>
#include "diskio.h"
#include "allocator.h"

char* loadDataFromDiskFile(char* fileName) {
    FILE* diskFile = fopen(fileName, "rb");

    if(diskFile == NULL)
        return NULL;

    char* diskData = (char*)malloc(sizeof(char) * DISK_SIZE);

    /* TODO: Return null if bytes read is less than expected*/
    if(!fread(diskData, sizeof(char), DISK_SIZE, diskFile)){
        if(diskFile)
            fclose(diskFile);
        free(diskData);
        return NULL;
    }

    fclose(diskFile);
    return diskData;
}

int saveDiskFile(char* fileName, char* data) {
    FILE* diskFile = fopen(fileName, "wb");

    if(!diskFile)
        return -1;

    if(!fwrite(data, sizeof(char), DISK_SIZE, diskFile)) {
        fclose(diskFile);
        return -1;
    }

    fclose(diskFile);
    return 1;
}

void freeDiskData(char* diskData) {
    free(diskData);
}

char* getSectorData(char* data, int tracknum, int sectornum) {
    return &data[
        ((tracknum & TRACK_ID_MASK) * TRACK_SIZE) +
        ((sectornum & SECTOR_ID_MASK) * SECTOR_SIZE)
    ];
}
