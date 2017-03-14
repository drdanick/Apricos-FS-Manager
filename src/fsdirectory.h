#ifndef FSDIRECTORY_H
#define FSDIRECTORY_H

#include "filesystem.h"

long long createDirectory(Filesystem* fs);
int createDirectoryAtBlock(Filesystem* fs, unsigned int blockNum);
int pushDirectoryToStack(Filesystem* fs, FsDirectory dir);
FsDirectory popDirectoryFromStack(Filesystem* fs);
FsDirectory openBlockAsDirectory(Filesystem* fs, unsigned int blockNum, char* dirName);
int isDirEntryFree(FsDirectoryEntry* entry);
int findNextFreeDirEntry(FsDirectory* dir);
FsDirectoryEntry* findDirEntryByName(FsDirectory* dir, char* name);
int removeDirEntry(FsDirectoryEntry* entry);
int removeDirEntryByName(FsDirectory* dir, char* name);
int addDirectoryBlockEntrytoDirectory(FsDirectory* parentDir, unsigned int childBlock, char* childName);
int addDirectoryEntrytoDirectory(FsDirectory* parentDir, int childtrack, int childSector, char* childName);

#endif /* FSDIRECTORY_H */
