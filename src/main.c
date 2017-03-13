#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "apricosfsman.h"
#include "commandproc.h"

unsigned long buffersize = INITIAL_INPUT_BUFFER_SIZE;
char* inputbuff;

Filesystem* globalFileSystem = NULL;

void cleanup() {
    /* free resources */
    free(inputbuff);

    if(globalFileSystem != NULL) {
        printf("Unmounting without saving...\n");
        unmountFilesystem(globalFileSystem, 0);
    }
}

void termhandler(int sig) {
    if(sig == SIGINT){
        cleanup();
        exit(EXIT_SUCCESS);
    }
}

void init(int argc, char** argv) {
    argc=argc; /* Satisfy compiler warnings until these are actually used */
    argv=argv;

    signal(SIGINT, termhandler);

    inputbuff = (char*)malloc(sizeof(char) * buffersize);
}

void printPrompt() {
    if(globalFileSystem) {
        char* volname = globalFileSystem->volumeInfo->volumeName;
        if(strlen(volname) < 1)  {
            printf("[Unformatted]");
        } else {
            printf("[%s", volname);
            printPathString(globalFileSystem);
            printf("]");
        }
    }
    printf(PROMPT);
}

void inputLoop() {
    do {
        printPrompt();
    } while(
            getline(&inputbuff, &buffersize, stdin) != -1
            && processLine(inputbuff) != -1);
}

int main(int argc, char** argv) {
    init(argc, argv);

    inputLoop();

    cleanup();

    printf("Bye!\n");
    return EXIT_SUCCESS;
}
