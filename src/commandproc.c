#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "commandproc.h"

char* strToLower(char* str) {
    int i = 0;
    for( ; str[i] != '\0'; i++)
        str[i] = tolower(str[i]);

    return str;
}

int processLine(char* line) {
    char* command;
    strToLower(line);
    command = strtok(line, " \n");

    if(command == NULL) {
        printf("error parsing input!\n");
        return -1;
    }

    if(strcmp("exit", command) == 0) {
        return -1;
    }
    else {
        printf("Unknown Command\n");
    }

    return 1;
}
