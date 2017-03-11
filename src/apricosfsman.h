#ifndef APRICOSFSMAN_H
#define APRICOSFSMAN_H

#include "filesystem.h"

/* misc. macros */

#define MIN(a, b) ((a < b) ? a : b)


/* console input related constants */

#define INITIAL_INPUT_BUFFER_SIZE 256
#define PROMPT "> "

/*
 * Globals
 */

extern Filesystem* globalFileSystem;


#endif /* APRICOSFSMAN_H */
