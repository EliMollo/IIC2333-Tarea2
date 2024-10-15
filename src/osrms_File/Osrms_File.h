#pragma once
#include <stdint.h>

#pragma pack(pop)
typedef struct osrmsFile {
    unsigned char validationByte;
    char fileName[14];
    unsigned int fileSize;
    unsigned int virtualAddress;
    char mode;
    char firstOrderTable[128];
} osrmsFile;
#pragma pack(pop)
