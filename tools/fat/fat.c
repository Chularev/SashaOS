#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t bool;
#define true 1
#define false 0

typedef struct 
{
    uint8_t BootJumpInstruction[3];
    uint8_t OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntryCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint32_t VolumeId;         
    uint8_t VolumeLabel[11]; 
    uint8_t SystemId[8];

} __attribute__((packed)) Header;

Header g_Header;
bool readHeader(FILE* disk)
{
    return fread(&g_Header, sizeof(g_Header), 1, disk) > 0;
}

bool readSectors(FILE* disk, uint32_t lba, uint32_t count, void* bufferOut)
{
    bool ok = true;
    ok = ok && (fseek(disk, lba * g_Header.BytesPerSector, SEEK_SET) == 0);
    ok = ok && (fread(bufferOut, g_Header.BytesPerSector, count, disk) == count);
    return ok;
}


uint8_t* g_Fat = NULL;
bool readFat(FILE* disk)
{
    g_Fat = (uint8_t*) malloc(g_Header.SectorsPerFat * g_Header.BytesPerSector);
    return readSectors(disk, g_Header.ReservedSectors, g_Header.SectorsPerFat, g_Fat);
}

typedef struct 
{
    // 8 bites file name
    // 3 bites extension
    // 8 + 3 = 11
    uint8_t Name[11];
    uint8_t Attributes;
    uint8_t _Reserved;
    uint8_t CreatedTimeTenths;
    uint16_t CreatedTime;
    uint16_t CreatedDate;
    uint16_t AccessedDate;
    uint16_t FirstClusterHigh;
    uint16_t ModifiedTime;
    uint16_t ModifiedDate;
    uint16_t FirstClusterLow;
    uint32_t Size;
} __attribute__((packed)) DirectoryEntry;

DirectoryEntry* g_RootDirectory = NULL;
uint32_t g_RootDirectoryEnd;

bool readRootDirectory(FILE* disk)
{
    uint32_t lba = g_Header.ReservedSectors + g_Header.SectorsPerFat * g_Header.FatCount;
    uint32_t size = sizeof(DirectoryEntry) * g_Header.DirEntryCount;
    uint32_t sectors = (size / g_Header.BytesPerSector);
    if (size % g_Header.BytesPerSector > 0)
        sectors++;

    g_RootDirectoryEnd = lba + sectors;
    g_RootDirectory = (DirectoryEntry*) malloc(sectors * g_Header.BytesPerSector);
    return readSectors(disk, lba, sectors, g_RootDirectory);
}


DirectoryEntry* findFile(const char* name)
{
    for (uint32_t i = 0; i < g_Header.DirEntryCount; i++)
    {
        // 8 bites file name
        // 3 bites extension
        // 8 + 3 = 11
        if (memcmp(name, g_RootDirectory[i].Name, 11) == 0)
            return &g_RootDirectory[i];
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Syntax: %s <disk image> <file name>\n", argv[0]);
        return -1;
    }

    FILE* disk = fopen(argv[1], "rb");
    if (!disk) {
        fprintf(stderr, "Cannot open disk image %s!\n", argv[1]);
        return -1;
    }

    if (!readHeader(disk)) {
        fprintf(stderr, "Could not read boot sector!\n");
        return -2;
    }

    if (!readFat(disk)) {
        fprintf(stderr, "Could not read FAT!\n");
        free(g_Fat);
        return -3;
    }

     if (!readRootDirectory(disk)) {
        fprintf(stderr, "Could not read Root Directory!\n");
        free(g_Fat);
        free(g_RootDirectory);
        return -4;
    }

    DirectoryEntry* fileEntry = findFile(argv[2]);
    if (!fileEntry) {
        fprintf(stderr, "Could not find file %s!\n", argv[2]);
        free(g_Fat);
        free(g_RootDirectory);
        return -5;
    }

    return 0;
}
