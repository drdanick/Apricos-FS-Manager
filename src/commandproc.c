#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include "commandproc.h"
#include "apricosfsman.h"
#include "filesystem.h"
#include "memdumper.h"
#include "allocator.h"
#include "diskio.h"
#include "fsdirectory.h"

char* strToLower(char* str) {
    int i = 0;
    for( ; str[i] != '\0'; i++)
        str[i] = tolower(str[i]);

    return str;
}

char* strToUpper(char* str) {
    int i = 0;
    for( ; str[i] != '\0'; i++)
        str[i] = toupper(str[i]);

    return str;
}

void printStatus() {
    static char cwd[4096]; /* Buffer for current working dir */

    getcwd(cwd, sizeof(cwd));

    printf("Program status:\n");
    printf("\tWorking Directory: %s\n", cwd);
    printf("\tDisk Image mounted: %s\n", globalFileSystem == NULL ? "None" : globalFileSystem->diskImagePath);

    if(globalFileSystem) {
        char* volname = globalFileSystem->volumeInfo->volumeName;
        printf("\tVolume name: %s\n", strlen(volname) < 1 ? "[Unformatted]" : volname);
        if(strlen(volname) >= 1) {
            printf("\t\tCurrent working directory: "); printPathString(globalFileSystem);
        }
    }

    printf("\n");
}

void mountCmd(char* args) {
    Filesystem* fs;

    if(args == NULL) {
        printf("Invalid file name\n");
        return;
    }

    fs = mountFilesystem(args);
    if(fs == NULL) {
        printf("Cannot load file \"%s\"\n", args);
        return;
    }

    globalFileSystem = fs;
    printf("mounted!\n");
}

void unmountCmd() {
    if(globalFileSystem == NULL) {
        printf("No FileSystem mounted!\n");
        return;
    }

    unmountFilesystem(globalFileSystem, 1);
    globalFileSystem = NULL;
    printf("unmounted!\n");
}

void peekCmd(unsigned int track, unsigned int sector) {
    char* peekData;
    char* peekEnd;

    if(globalFileSystem == NULL) {
        printf("No FileSystem mounted\n");
        return;
    }

    peekData = getSectorData(globalFileSystem->diskData, track, sector);
    peekEnd = &peekData[SECTOR_SIZE];

    do {
        int i = 0;
        char* asciiData = peekData;
        printf("%03u  ", SECTOR_SIZE - (int)(peekEnd-peekData));

        for( ; i < PEEK_COLUMNS_PER_LINE; i++) {
            dumpMemoryAsHex(peekData, peekEnd, PEEK_BYTES_PER_COLUMN);

            peekData += PEEK_BYTES_PER_COLUMN;
            printf("  ");
        }
        printf("[");
        dumpMemoryAsASCII(asciiData, peekEnd, PEEK_BYTES_PER_COLUMN * PEEK_COLUMNS_PER_LINE);
        printf("]\n");
    } while(peekData < peekEnd);

}

void mkdirCmd(char* dirName) {
    FsDirectory* dir = getWorkingDirectory(globalFileSystem);

    if(!dirName || !dir) {
        return;
    }

    dirName = strToUpper(dirName);

    if(findDirEntryByName(dir, dirName)) {
        printf("Directory already exists!\n");
        return;
    }

    allocateAndAddDirectoryEntryToDirectory(globalFileSystem, dir, dirName);
}

void lsCmd() {
    if(!globalFileSystem || !getWorkingDirectory(globalFileSystem)) {
        return;
    }

    printf("Directory listing of ");
    printPathString(globalFileSystem);
    printf(":\n");
    printDirectoryListing(getWorkingDirectory(globalFileSystem), "\t");
}

int processLine(char* line) {
    char* command;
    strToLower(line);
    command = strtok(line, " \n");

    if(command == NULL) {
        command = "";
    }

    /* Execute the command */

    if(strcmp("exit", command) == 0) {
        return -1;
    }
    else if(strcmp("status", command) == 0) {
        printStatus();
    }
    else if(strcmp("mount", command) == 0) {
        mountCmd(strtok(NULL, " \n"));
    }
    else if(strcmp("unmount", command) == 0) {
        unmountCmd();
    }
    else if(strcmp("peek", command) == 0) {
        char* peekarg;
        unsigned int track;
        unsigned int sector;

        peekarg = strtok(NULL, "\n");

        if(peekarg == NULL || sscanf(peekarg, "%u %u", &track, &sector) < 2) {
            printf("Argument size missmatch!\n");
        } else {
            if(track >= TRACKS || sector >= SECTORS_PER_TRACK) {
                printf("Invalid track/sector number\n");
            } else {
                peekCmd(track, sector);
            }
        }
    }
    else if(strcmp("format", command) == 0) {
        char* volnamearg = strtok(NULL, "\n");
        if(!globalFileSystem) {
            printf("No FileSystem mounted\n");
        } else {
            if(volnamearg == NULL) {
                printf("Missing volume name\n");
            } else if(strlen(volnamearg) > VOLUME_NAME_LENGTH) {
                printf("Given name exceeds maximum length of %d\n", VOLUME_NAME_LENGTH);
            } else if(globalFileSystem) {
                printf("Formatting... ");
                formatFilesystem(globalFileSystem, volnamearg);
                printf("Done!\n");
            }
        }
    }
    else if(strcmp("mkdir", command) == 0) {
        char* dirName = strtok(NULL, " \n");
        if(dirName) {
            mkdirCmd(dirName);
        } else {
            printf("Invalid directory name.\n");
        }
    }
    else if(strcmp("ls", command) == 0 || strcmp("dir", command) == 0) {
        if(globalFileSystem) {
            lsCmd();
        } else {
            printf("No filesystem mounted!\n");
        }
    }
    else {
        printf("Unknown Command\n");
    }

    return 1;
}
