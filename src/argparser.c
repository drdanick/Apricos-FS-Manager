#include "argparser.h"
#include "apricosfsman.h"

#include <stdlib.h>
#include <stdio.h>

const struct option long_options[] = {
    {"version", no_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

const char* option_descriptions[] = {
    "Print version information and exit",
    "Print this message and exit",
};

void printHelp() {
    int i = 0;
    struct option coption;
    printf("Aprsim usage:\n");
    printf("\tapricosfsman [options] [diskimage]\n");
    printf("Options:\n");
    while(coption = long_options[i],
            !(coption.flag == 0
            && coption.has_arg == 0
            && coption.val == 0
            && coption.name == 0)) {
        printf("\t--%-20s -%-5c %s\n", coption.name, coption.val, option_descriptions[i]);
        i++;
    }
}

Settings getSettingsFromArgs(int argc, char** argv) {
    Settings s;
    int c;

    s.diskImage = NULL;

    while(c = getopt_long(argc, argv, "vh", long_options, NULL), c != -1) {
        switch(c) {
            case 'v':
                printf("Apricos FS Manager V%s\n", VERSION);
                exit(EXIT_SUCCESS);
                break;
            case 'h':
                printHelp();
                exit(EXIT_SUCCESS);
                break;
            case '?':
            default:
                exit(EXIT_FAILURE);
                break;
        }
    }

    /* TODO: Set disk image if it's in the arg list */

    return s;
}
