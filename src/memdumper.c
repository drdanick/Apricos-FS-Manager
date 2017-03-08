#include <stdio.h>
#include "memdumper.h"

static const char HEX[] = {
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F',
};

void byteToHex(char byte, char* output) {
    output[0] = HEX[(byte >> 4) & 0x0F];
    output[1] = HEX[byte & 0x0F];
}

void dumpMemoryAsHex(char* data, char* dataEnd, int count) {
    static char hexbuff[2];
    int i = 0;
    while(i < count) {
        if(&data[i] >= dataEnd)
            break;
        byteToHex(data[i++], hexbuff);
        printf("%s ", hexbuff);
    }

    /* Print padding */
    for( ; i < count; i++) {
        printf(".. ");
    }
}

void dumpMemoryAsASCII(char* data, char* dataEnd, int count) {
    int i = 0;
    while(i < count) {
        char c;

        if(&data[i] >= dataEnd)
            break;

        c = data[i++];
        printf("%c", (c < ' ' || c == (char)(127)) ? '.' : c);
    }

    /* Print padding */
    for( ; i < count; i++) {
        printf(".");
    }
}
