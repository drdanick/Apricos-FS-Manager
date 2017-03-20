#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "diskio.h"
#include "allocator.h"

char* loadDataFromDiskFile(char* fileName) {
    char* diskData;
    long long bytesRead;
    FILE* diskFile = fopen(fileName, "rb");

    if(diskFile == NULL)
        return NULL;

    diskData = createDiskData();
    bytesRead = fread(diskData, sizeof(char), DISK_SIZE, diskFile);

    if(!bytesRead || bytesRead != DISK_SIZE){
        if(diskFile)
            fclose(diskFile);
        free(diskData);
        return NULL;
    }

    fclose(diskFile);
    return diskData;
}

char* createDiskData() {
    char* data = (char*)malloc(sizeof(char) * DISK_SIZE);

    if(data) {
        memset(data, '\0', DISK_SIZE);
    }

    return data;
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

char* getBlockData(char* data, int blockNum) {
    return getSectorData(data, BLOCK_TO_TRACK(blockNum), BLOCK_TO_SECTOR(blockNum));
}
