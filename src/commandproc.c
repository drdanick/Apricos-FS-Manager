#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include "commandproc.h"
#include "apricosfsman.h"
#include "filesystem.h"

unsigned int HEX_TO_DEC[] = {
    0, 1, 2, 3,
    4, 5, 6, 7,
    8, 9, 0, 0,
    0, 0, 0, 0,
    0, 0x0A, 0x0B, 0x0C,
    0x0D, 0x0E, 0x0F
};

long long hexToDec(char* hexstr) {
    long long output = 0;
    unsigned int length = strlen(hexstr);

    while(length > 0) {
        if((int)(hexstr[length - 1] - '0') > 22)
            return -1;
        output <<= 4;
        output |= HEX_TO_DEC[(int)(hexstr[--length] - '0')];

    }

    return output;
}

char* strToUpper(char* str) {
    int i = 0;
    for( ; str[i] != '\0'; i++)
        str[i] = toupper(str[i]);

    return str;
}

char* strToLower(char* str) {
    int i = 0;
    for( ; str[i] != '\0'; i++)
        str[i] = tolower(str[i]);

    return str;
}

void printStatus() {
    static char cwd[4096]; /* Buffer for current working dir */

    getcwd(cwd, sizeof(cwd));

    printf("Program status:\n");
    printf("\tWorking Directory: %s\n", cwd);
    printf("\tDisk Image mounted: %s\n", globalFileSystem == NULL ? "None" : globalFileSystem->diskImagePath);

    printf("\n");
}

void mountCmd(char* args) {
    Filesystem* fs = mountFilesystem(args);
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

void peekCommand(long long address, long count) {
    /* TODO */
    printf("Will peek %ld bytes @ address %lld\n", count, address);
}

int processLine(char* line) {
    char* command;
    strToLower(line);
    command = strtok(line, " \n");

    if(command == NULL) {
        printf("error parsing input!\n");
        return -1;
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
        static char hexbuff[32];
        char* peekarg;
        long long hexAddress;
        unsigned long count;

        peekarg = strtok(NULL, "\n");

        if(peekarg == NULL || sscanf(peekarg, "%s %ld", hexbuff, &count) < 2) {
            printf("Argument size missmatch!\n");
        } else {
            hexAddress = hexToDec(strToUpper(hexbuff));

            if(hexAddress != -1) {
                peekCommand(hexAddress, count);
            } else {
                printf("Invalid address!\n");
            }
        }
    }
    else {
        printf("Unknown Command\n");
    }

    return 1;
}
