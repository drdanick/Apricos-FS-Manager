#ifndef FSFILE_H
#define FSFILE_H

#include "filesystem.h"

long long createFile(Filesystem* fs, FsDirectory* parentDir, char* name);
int createFileAtBlock(Filesystem* fs, FsDirectory* parentDir, unsigned int blockNum, char* name);

#endif /* FSFILE_H */
