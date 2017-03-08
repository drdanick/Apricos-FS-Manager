#ifndef MEMDUMPER_H
#define MEMDUMPER_H

void byteToHex(char byte, char* output);
void dumpMemoryAsHex(char* data, char* dataEnd, int count);
void dumpMemoryAsASCII(char* data, char* dataEnd, int count);

#endif /* MEMDUMPER_H */
