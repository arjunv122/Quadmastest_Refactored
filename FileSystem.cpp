#include "FileSystem.h"
#include "TimeManager.h"

#define MAX_FILES 20

FileEntry fileList[MAX_FILES];
int fileCount = 0;

String testCreatedFiles[20];
int testCreatedCount = 0;

int extractDateFromFilename(const String &filename) {
    String base = filename;

    // Remove leading "/"
    if (base.startsWith("/")) {
        base = base.substring(1);
    }

    // Remove ". txt" extension
    int dotIndex = base.indexOf('.');
    if (dotIndex > 0) {
        base = base.substring(0, dotIndex);
    }

    // ✅ FIX: Handle variable length filenames (DMYYYY, DMMYYYY, DDMYYYY, DDMMYYYY)
    int len = base.length();
    
    if (len < 6 || len > 8) {
        Serial.printf("Bad date filename format (length %d): %s\n", len, filename.c_str());
        return 0;
    }

    String day, month, year;

    // Parse based on length
    if (len == 8) {
        // DDMMYYYY (e.g., 04112025)
        day = base.substring(0, 2);
        month = base.substring(2, 4);
        year = base.substring(4, 8);
    } 
    else if (len == 7) {
        // Could be DMMYYYY or DDMYYYY
        // Check first two chars to determine
        int firstTwo = base.substring(0, 2).toInt();
        
        if (firstTwo <= 31 && firstTwo >= 10) {
            // Likely DDMYYYY (day 10-31, single digit month)
            day = base.substring(0, 2);
            month = base.substring(2, 3);
            year = base.substring(3, 7);
        } else {
            // Likely DMMYYYY (single digit day)
            day = base.substring(0, 1);
            month = base.substring(1, 3);
            year = base.substring(3, 7);
        }
    }
    else if (len == 6) {
        // DMYYYY (e.g., 612026 = 6/1/2026)
        day = base.substring(0, 1);
        month = base.substring(1, 2);
        year = base.substring(2, 6);
    }
    else {
        Serial.printf("Unexpected length %d for:  %s\n", len, filename.c_str());
        return 0;
    }

    // Pad day and month with leading zeros if needed
    if (day.length() == 1) day = "0" + day;
    if (month.length() == 1) month = "0" + month;

    // Convert to YYYYMMDD integer for sorting
    String yyyymmdd = year + month + day;
    int dateNum = yyyymmdd.toInt();
    
    Serial.printf("Parsed '%s':  Day=%s, Month=%s, Year=%s → %d\n", 
                 filename.c_str(), day.c_str(), month.c_str(), year.c_str(), dateNum);
    
    return dateNum;
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
    Serial.printf("\n=== LISTING ALL FILES IN SPIFFS ===\n");

    File root = fs.open(dirname);
    if (!root) {
        Serial.println("- Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("- Not a directory");
        return;
    }

    fileCount = 0;
    File file = root.openNextFile();
    while (file) {
        if (! file.isDirectory() && fileCount < MAX_FILES) {
            fileList[fileCount].filename = String(file.name());
            fileList[fileCount].dateNumeric = extractDateFromFilename(fileList[fileCount].filename);
            fileCount++;
        }
        file = root.openNextFile();
    }

    Serial.printf("\n=== TOTAL FILES STORED: %d ===\n", fileCount);
}

void sortFilesByDate() {
    if (fileCount > 1) {
        for (int i = 0; i < fileCount - 1; i++) {
            for (int j = i + 1; j < fileCount; j++) {
                if (fileList[i].dateNumeric < fileList[j].dateNumeric) {
                    FileEntry temp = fileList[i];
                    fileList[i] = fileList[j];
                    fileList[j] = temp;
                }
            }
        }
    }
}

void checkSPIFFSStorage() {
    Serial.printf("\nTotal SPIFFS Storage: %d bytes\n", SPIFFS.totalBytes());
    Serial.printf("Used SPIFFS Storage: %d bytes\n", SPIFFS.usedBytes());
    Serial.printf("Free SPIFFS Storage: %d bytes\n", SPIFFS.totalBytes() - SPIFFS.usedBytes());
}

void manageStorageCapacity(fs:: FS &fs) {
    Serial.println("\n=== STORAGE CAPACITY MANAGER RUNNING ===");

    size_t total = SPIFFS.totalBytes();
    size_t used = SPIFFS.usedBytes();

    if (total == 0) {
        Serial.println("Warning:  SPIFFS total size is zero.  Cannot manage storage.");
        return;
    }

    float usagePercent = (float)used / total * 100.0f;
    int filesDeleted = 0;

    Serial.printf("Current Usage: %u bytes / %u bytes (%.2f%%)\n", used, total, usagePercent);

    if (usagePercent > SPIFFS_CAPACITY_LIMIT_PERCENT) {
        Serial.printf("❌ Capacity limit (%d%%) exceeded!  Deleting oldest files...\n",
                      SPIFFS_CAPACITY_LIMIT_PERCENT);

        for (int i = fileCount - 1; i >= 0; i--) {
            used = SPIFFS.usedBytes();
            usagePercent = (float)used / total * 100.0f;

            if (usagePercent <= SPIFFS_CAPACITY_LIMIT_PERCENT) {
                break;
            }

            String oldestPath = fileList[i].filename;

            if (fs.remove(oldestPath)) {
                Serial.printf("✅ Deleted oldest file: %s\n", oldestPath.c_str());
                filesDeleted++;
            } else {
                Serial.printf("❌ Failed to delete file: %s\n", oldestPath.c_str());
            }
        }
    }

    if (filesDeleted > 0) {
        Serial.printf("Total files deleted: %d.  Rerunning listDir and sortFilesByDate.\n", filesDeleted);
        listDir(SPIFFS, "/", 0);
        sortFilesByDate();
        checkSPIFFSStorage();
    } else {
        Serial.println("Storage usage is acceptable. No files deleted.");
    }
}

void prepareFilesForWrite() {
    if (! SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed!  Cannot prepare files.");
        return;
    }

    listDir(SPIFFS, "/", 0);
    sortFilesByDate();
    manageStorageCapacity(SPIFFS);
}

void fileWrite(ElectricalConductivity elCdvty, float *temperatures, uint32_t chipId) {
    // -------- Time handling (UTC → IST) --------
    DateTime nowUTC = rtc.now();
    DateTime nowIST = DateTime(nowUTC.unixtime() + IST_OFFSET_SECONDS);

    String time = String(nowIST.timestamp(DateTime::TIMESTAMP_TIME));
    currentTime = time;
    currentDate = String(nowIST.day()) + "-" +
                  String(nowIST.month()) + "-" +
                  String(nowIST.year());

    // Example: 07 Jan 2026 → "07012026"
    String formattedDate = two(nowIST. day()) + two(nowIST.month()) + String(nowIST.year());
    String fileName = "/" + formattedDate + ".txt";

    // -------- SPIFFS --------
    if (!SPIFFS.begin()) {
        Serial.println("SPIFFS mount failed");
        return;
    }

    File file = SPIFFS.open(fileName, "a");
    if (!file) {
        Serial.println("Error opening file for writing");
        return;
    }

    // ✅ ORIGINAL FORMAT - WITH PROPER LABELS AND SPACING
    file.println("File Name: " + String(file.name()));
    file.println("Hardware No.: " + String(chipId));
    file.println("Test Count: " + String(testsCounter));
    file.println("RFID No.: " + rfidNumber);
    file.println("Time: " + time);  // IST
    file.println("Battery%: " + String(battPercentage));

    // -------- EC (compensated values only) --------
    file.println(
        "EC: " +
        String(elCdvty.ecWithComp[0]) + ", " +
        String(elCdvty.ecWithComp[1]) + ", " +
        String(elCdvty.ecWithComp[2]) + ", " +
        String(elCdvty.ecWithComp[3])
    );

    // -------- Temperature --------
    file. println(
        "Temperature: " +
        String(temperatures[0]) + ", " +
        String(temperatures[1]) + ", " +
        String(temperatures[2]) + ", " +
        String(temperatures[3])
    );

    // -------- Clinical --------
    file.print("Clinical: ");
    for (int j = 0; j < 4; j++) {
        if (positives[0][j] != "") {   // RED / Clinical
            file.print(positives[0][j] + " ");
        }
    }
    file.println();

    // -------- Sub Clinical --------
    file.print("Sub Clinical: ");
    for (int j = 0; j < 4; j++) {
        if (positives[1][j] != "") {   // YELLOW / Subclinical
            file.print(positives[1][j] + " ");
        }
    }
    file.println();

    // -------- Separator between tests --------
    file.println();

    Serial.print("File size: ");
    Serial.println(file. size());

    file.close();

    // -------- Debug:  read back file --------
    File file2 = SPIFFS. open(fileName, "r");
    if (!file2) {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.println("filename is: " + String(file2.name()));
    while (file2.available()) {
        Serial.write(file2.read());
    }
    file2.close();

    Serial.println("total bytes: " + String(SPIFFS.totalBytes()));
    Serial.println("used bytes: " + String(SPIFFS.usedBytes()));
}
String readFile(fs::FS &fs, const char *path) {
    Serial.printf("Reading file: %s\r\n", path);

    String filePath = String(path);
    if (!filePath.startsWith("/")) {
        filePath = "/" + filePath;
    }

    if (! fs.exists(filePath)) {
        Serial.printf("❌ File not found: %s\n", filePath.c_str());
        return "";
    }

    File file = fs.open(filePath, FILE_READ);
    if (!file || file.isDirectory()) {
        Serial.printf("❌ Failed to open file for reading: %s\n", filePath.c_str());
        return "";
    }

    String content = "";
    while (file.available()) {
        content += (char)file.read();
    }

    file.close();
    return content;
}

void addContentToAllFiles(fs::FS &fs) {
    Serial.println("\n=== ADDING CONTENT TO ALL FILES ===");

    for (int i = 0; i < fileCount; i++) {
        String filePath = fileList[i].filename;

        if (!filePath.startsWith("/")) {
            filePath = "/" + filePath;
        }

        File file = fs.open(filePath, FILE_APPEND);

        if (!file) {
            Serial.printf("File %s missing. Creating.. .\n", filePath.c_str());
            file = fs.open(filePath, FILE_WRITE);
        }

        if (file) {
            file.println("Additional test data added.");
            file.close();
            Serial.printf("Updated file: %s\n", filePath. c_str());
        } else {
            Serial.printf("❌ Failed to open file for writing: %s\n", filePath.c_str());
        }
    }
}

void test_Sort_10RandomFiles() {
    Serial.println("\n=== CREATING 10 TEST FILES (NOV 2024 – FEB 2025) ===");

    testCreatedCount = 0;

    const char *testFiles[] = {
        "/05112024.txt",
        "/18112024.txt",
        "/30112024.txt",
        "/05122024.txt",
        "/19122024.txt",
        "/04012025.txt",
        "/17012025.txt",
        "/29012025.txt",
        "/11022025.txt",
        "/23022025.txt"
    };

    const int N = 10;

    for (int i = 0; i < N; i++) {
        String path = String(testFiles[i]);

        SPIFFS.remove(path);
        File f = SPIFFS.open(path, FILE_WRITE);

        if (!f) {
            Serial.printf("❌ Failed to create %s\n", path.c_str());
            continue;
        }

        f.println("FileName " + path);
        f.println("RFIDno 12345");
        f.println("HardwareNo IQ001");
        f.println("TestCount " + String(i + 1));
        f.println("time 2025-02-10 12:00:00");
        f.println("Battery% 87");
        f.println("ECwComp 1.1, 2.2, 3.3, 4.4");
        f.println("Postives {FL}{FR}");
        f.close();

        testCreatedFiles[testCreatedCount++] = path;
        Serial. printf("Created test file: %s\n", path.c_str());
    }

    Serial.printf("Total test files created: %d\n", testCreatedCount);
}

void deleteTestFiles() {
    Serial.println("\n=== DELETING TEST FILES CREATED FOR SORT/PRINT TEST ===");

    for (int i = 0; i < testCreatedCount; i++) {
        String path = testCreatedFiles[i];
        if (SPIFFS.exists(path)) {
            SPIFFS.remove(path);
            Serial.printf("Deleted: %s\n", path.c_str());
        } else {
            Serial.printf("Not found (already deleted? ): %s\n", path.c_str());
        }
    }

    testCreatedCount = 0;
    Serial.println("=== TEST FILE CLEANUP COMPLETE ===");
}

void testStorageCapacityManager() {
    Serial.println("\n==============================================");
    Serial.println("=== 🔬 TESTING manageStorageCapacity() ===");
    Serial.println("==============================================");

    if (!SPIFFS.begin(true)) {
        Serial.println("❌ SPIFFS Mount Failed!  Aborting test.");
        return;
    }

    deleteTestFiles();
    SPIFFS.remove("/BIGFILL.bin");

    test_Sort_10RandomFiles();

    listDir(SPIFFS, "/", 0);
    sortFilesByDate();

    String oldestFileBeforeTest = (fileCount > 0) ? fileList[fileCount - 1].filename : "";

    File big = SPIFFS.open("/BIGFILL.bin", FILE_WRITE);
    if (big) {
        const size_t chunkSize = 1024;
        uint8_t buf[chunkSize];
        memset(buf, 0xAA, chunkSize);
        for (int i = 50; i > 0; i--) {
            big.write(buf, chunkSize);
        }
        big. close();
        Serial.println("Created BIGFILL.bin to artificially raise usage.");
    } else {
        Serial.println("⚠️ Could not create BIGFILL.bin.  Test may not trip threshold.");
    }

    Serial.println("\n--- Initial State Before Manager Run ---");
    checkSPIFFSStorage();
    Serial.printf("File Count: %d.  Oldest File: %s\n", fileCount, oldestFileBeforeTest. c_str());

    manageStorageCapacity(SPIFFS);

    Serial.println("\n--- Verification of Deletion ---");

    if (SPIFFS.exists("/BIGFILL.bin")) {
        if (! SPIFFS.exists(oldestFileBeforeTest)) {
            Serial.println("✅ PASS: The intended oldest file was deleted by the manager.");
        } else {
            Serial.printf("❌ FAIL:  Oldest file %s still exists.  Manager failed to clear space.\n", oldestFileBeforeTest.c_str());
        }
    } else {
        Serial.println("❌ FAIL: BIGFILL.bin disappeared.  Cannot confirm usage state.");
    }

    listDir(SPIFFS, "/", 0);
    checkSPIFFSStorage();

    Serial.println("\n--- Cleaning Up ---");
    SPIFFS.remove("/BIGFILL. bin");
    deleteTestFiles();
    listDir(SPIFFS, "/", 0);
    checkSPIFFSStorage();
    Serial.println("=== TEST COMPLETE ===");
}