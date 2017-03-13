#ifndef FSDIRECTORY_H
#define FSDIRECTORY_H

#include "filesystem.h"

long long createDirectory(Filesystem* fs);
int createDirectoryAtBlock(Filesystem* fs, unsigned int blockNum);
int pushDirectoryToStack(Filesystem* fs, FsDirectory dir);
FsDirectory popDirectoryFromStack(Filesystem* fs);
FsDirectory openBlockAsDirectory(Filesystem* fs, unsigned int blockNum, char* dirName);

#endif /* FSDIRECTORY_H */
