#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <unistd.h>
#include <getopt.h>

typedef struct {
    char* diskImage;
} Settings;


Settings getSettingsFromArgs(int argc, char** argv);

#endif /* ARGPARSER_H */
