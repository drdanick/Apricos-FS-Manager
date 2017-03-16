#ifndef FSDIRECTORY_H
#define FSDIRECTORY_H

#include "filesystem.h"

long long createDirectory(Filesystem* fs);
int createDirectoryAtBlock(Filesystem* fs, unsigned int blockNum);
int pushDirectoryToStack(Filesystem* fs, FsDirectory dir);
FsDirectory popDirectoryFromStack(Filesystem* fs);
int openBlockAsDirectory(Filesystem* fs, unsigned int blockNum, char* dirName, FsDirectory* dir);
int getFsDirectoryFromEntry(Filesystem* fs, FsDirectoryEntry* entry, FsDirectory* dir);
void printDirectoryEntryAttributes(FsDirectoryEntry* entry, char* prefix, char* suffix);
void printDirectoryListing(FsDirectory* dir, char* prefix);
int isDirEntryFree(FsDirectoryEntry* entry);
int findNextFreeDirEntry(FsDirectory* dir);
int countDirectoryEntries(FsDirectory* dir);
FsDirectoryEntry* findDirEntryByName(FsDirectory* dir, char* name);
int removeDirEntry(FsDirectoryEntry* entry);
int removeDirEntryByName(FsDirectory* dir, char* name);
int allocateAndAddEntryToDirectory(Filesystem* fs, FsDirectory* parentDir, char* childName, char isFile);
int addBlockEntrytoDirectory(FsDirectory* parentDir, unsigned int childBlock, char* childName, char isFile);
int addEntrytoDirectory(FsDirectory* parentDir, int childtrack, int childSector, char* childName, char isFile);
void directoryEntryToggleExecutableFlag(FsDirectoryEntry* entry);

#endif /* FSDIRECTORY_H */
