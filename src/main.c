#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "apricosfsman.h"
#include "commandproc.h"

unsigned long buffersize = INITIAL_INPUT_BUFFER_SIZE;
char* inputbuff;

void cleanup() {
    /* free resources */
    free(inputbuff);
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

void inputLoop() {
    do {
        printf(PROMPT);
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
