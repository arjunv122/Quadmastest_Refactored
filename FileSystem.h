#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "Config.h"

// Struct for file management
struct FileEntry {
    String filename;
    int dateNumeric;
};

extern FileEntry fileList[];
extern int fileCount;

// Function declarations
void fileWrite(ElectricalConductivity elCdvty, float *temperatures, uint32_t chipId);
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void sortFilesByDate();
void checkSPIFFSStorage();
void manageStorageCapacity(fs::FS &fs);
void prepareFilesForWrite();
int extractDateFromFilename(const String &filename);
void testStorageCapacityManager();
void deleteTestFiles();
void test_Sort_10RandomFiles();
String readFile(fs::FS &fs, const char *path);
void addContentToAllFiles(fs::FS &fs);

#endif