#ifndef FSFILE_H
#define FSFILE_H

#include "filesystem.h"

long long createFile(Filesystem* fs, FsDirectory* parentDir, char* name);
int createFileAtBlock(Filesystem* fs, FsDirectory* parentDir, unsigned int blockNum, char* name);
int openBlockAsFile(Filesystem* fs, unsigned int blockNum, char* fileName, FsFile* file);
int getFsFileFromEntry(Filesystem* fs, FsDirectoryEntry* entry, FsFile* file);
unsigned int calculateFileSize(Filesystem* fs, FsFile* file);
int deleteAllFileBlocks(Filesystem* fs, FsFile* file);

#endif /* FSFILE_H */
