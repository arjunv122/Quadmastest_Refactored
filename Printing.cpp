#include "Printing.h"
#include "FileSystem.h"
#include "TimeManager.h"

// Simply move the entire content from Thermal_PrinterforshreyanshTEMP.h here
// I'll include the key functions: 

BluetoothSerial SerialBT;
BLEScan *pBLEScan;

#define TARGET_NAME "MPT-II"
#define TARGET_NAME2 "SR588"
#define SCAN_DURATION 10

bool connected = false;
const char *pin = "0000";

// ✅ ADD THIS - Missing variable declaration
bool lastPrintHadTests = false;


void toMACAddress(const String &macStr, uint8_t *macArray) {
    int i = 0;
    int j = 0;
    while (i < macStr.length() && j < 6) {
        String byteString = macStr.substring(i, i + 2);
        macArray[j] = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
        i += 3;
        j++;
    }

    Serial.print("Converted MAC Address: {");
    for (int k = 0; k < 6; k++) {
        Serial.print("0x");
        if (macArray[k] < 0x10) {
            Serial.print("0");
        }
        Serial.print(macArray[k], HEX);
        if (k < 5)
            Serial.print(", ");
    }
    Serial.println("};");
}

bool connectToPrinter() {
    if (connected && SerialBT.connected()) {
        Serial.println("Printer already connected.  Ready to print.");
        return true;
    }

    Serial.println("\n=== STARTING PRINTER CONNECTION (Scan & Connect) ===");

    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);

    Serial.println("Scanning for devices...");
    BLEScanResults results = pBLEScan->start(SCAN_DURATION);
    String targetAddress = "";
    String target_name = "";

    for (int i = 0; i < results.getCount(); i++) {
        BLEAdvertisedDevice device = results.getDevice(i);
        if (device.getName() == TARGET_NAME) {
            target_name = TARGET_NAME;
            targetAddress = device.getAddress().toString().c_str();
            break;
        }
        if (device.getName() == TARGET_NAME2) {
            target_name = TARGET_NAME2;
            targetAddress = device.getAddress().toString().c_str();
            break;
        }
    }

    pBLEScan->stop();
    BLEDevice::deinit();
    delay(1000);

    if (targetAddress != "") {
        Serial.printf("Connecting to device %s at %s.. .\n", target_name, targetAddress.c_str());

        if (! SerialBT.begin("ESP32test", true)) {
            Serial.println("Failed to start BluetoothSerial.");
            connected = false;
            return false;
        }

        SerialBT.setPin(pin);
        uint8_t macArray[6];
        toMACAddress(targetAddress, macArray);

        connected = SerialBT.connect(macArray);

        if (connected) {
            Serial.println("Connected Successfully!");
            return true;
        } else {
            while (! SerialBT.connected(10000)) {
                Serial.println("Failed to connect. Restarting app may be required.");
                return false;
            }
        }
    } else {
        Serial.println("Printer not found!");
    }
    return connected;
}

bool tpsetup() {
    Serial.begin(115200);
    Serial.println("Initializing...");

    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);

    Serial.println("Scanning for devices.. .");
    BLEScanResults results = pBLEScan->start(SCAN_DURATION);

    String targetAddress = "";
    String target_name = "";

    for (int i = 0; i < results.getCount(); i++) {
        BLEAdvertisedDevice device = results.getDevice(i);
        Serial.printf("Found device: %s, Address: %s\n",
                      device.getName().c_str(),
                      device. getAddress().toString().c_str());

        if (device.getName() == TARGET_NAME) {
            target_name = TARGET_NAME;
            targetAddress = device. getAddress().toString().c_str();
            break;
        }
        if (device.getName() == TARGET_NAME2) {
            target_name = TARGET_NAME2;
            targetAddress = device.getAddress().toString().c_str();
            break;
        }
    }

    pBLEScan->stop();
    BLEDevice::deinit();
    delay(1000);

    if (targetAddress != "") {
        Serial.printf("Connecting to device %s at %s...\n", target_name, targetAddress.c_str());

        if (!SerialBT. begin("ESP32test", true)) {
            Serial.println("Failed to start BluetoothSerial.");
            return false;
        }

        SerialBT. setPin(pin);
        Serial.println("Bluetooth started.. .");
        uint8_t macArray[6];
        toMACAddress(targetAddress, macArray);

        connected = SerialBT.connect(macArray);
        if (connected) {
            Serial.println("Connected Succesfully!");
            return true;
        } else {
            while (!SerialBT.connected(10000)) {
                Serial. println("Failed to connect. Make sure remote device is available and in range, then restart app.");
                return false;
            }
        }

        return false;
    }
    return false;
}

void bleSetup() {
    Serial.println("\n=== INITIALIZING BLE CONNECTION ===");

    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    Serial.println("Scanning for BLE devices...");

    BLEScanResults results = pBLEScan->start(SCAN_DURATION);
    String targetAddress = "";
    String target_name = "";

    for (int i = 0; i < results.getCount(); i++) {
        BLEAdvertisedDevice device = results.getDevice(i);
        Serial.printf("Found device: %s, Address: %s\n", device.getName().c_str(), device.getAddress().toString().c_str());

        if (device.getName() == TARGET_NAME) {
            target_name = TARGET_NAME;
            targetAddress = device.getAddress().toString().c_str();
            break;
        }
        if (device. getName() == TARGET_NAME2) {
            target_name = TARGET_NAME2;
            targetAddress = device.getAddress().toString().c_str();
            break;
        }
    }

    pBLEScan->stop();
    BLEDevice:: deinit();
    delay(1000);

    if (targetAddress != "") {
        Serial.printf("Connecting to %s at %s...\n", target_name, targetAddress.c_str());

        if (!SerialBT.begin("ESP32test", true)) {
            Serial.println("Failed to start BluetoothSerial.");
            return;
        }

        SerialBT.setPin(pin);
        Serial.println("Bluetooth started.. .");
        uint8_t macArray[6];
        toMACAddress(targetAddress, macArray);
        connected = SerialBT. connect(macArray);

        if (connected) {
            Serial. println("Connected to printer.");
            printLast5Tests();
            return;
        } else {
            Serial.println("Failed to connect.");
        }
    } else {
        Serial.println("Printer not found!");
    }
}

String formatDateFromFilename(String fname) {
    // Expect "/DDMMYYYY.txt" or "DDMMYYYY.txt"
    if (fname.startsWith("/")) fname = fname.substring(1);

    int dot = fname.indexOf('.');
    if (dot > 0) fname = fname.substring(0, dot); // DDMMYYYY

    if (fname.length() != 8) return "";

    return fname.substring(0, 2) + "/" +
           fname.substring(2, 4) + "/" +
           fname.substring(4, 8);
}

void printLast5Tests() {
    Serial.println("\n=== 🖨️ PRINTING LAST 5 TESTS TO BLE PRINTER ===");

    const int testsNeeded = 5;
    int testsFound = 0;
    String testsToPrint[5];

    for (int i = 0; i < fileCount && testsFound < testsNeeded; i++) {

        Serial.printf("🔍 Reading file: %s\n", fileList[i]. filename. c_str());
        String content = readFile(SPIFFS, fileList[i]. filename. c_str());

        if (content.length() == 0) {
            Serial. println("⚠️ Empty or unreadable file, skipping");
            continue;
        }

        // -------- Split file into lines --------
        String lines[300];
        int lineCount = 0;
        int startIdx = 0;

        while (startIdx < content. length() && lineCount < 300) {
            int endIdx = content. indexOf('\n', startIdx);
            if (endIdx == -1) break;
            String line = content.substring(startIdx, endIdx);
            line.trim();
            startIdx = endIdx + 1;
            if (line.length() > 0) {
                lines[lineCount++] = line;
            }
        }

        Serial.printf("   Found %d lines in file\n", lineCount);

        // -------- Parse tests bottom-up --------
        for (int j = lineCount - 1; j >= 0 && testsFound < testsNeeded; j--) {

            // ✅ SUPPORT BOTH FORMATS
            bool isNewFormat = lines[j].startsWith("File Name:");
            bool isOldFormat = lines[j].startsWith("FileName");
            
            if (! isNewFormat && !isOldFormat) continue;

            Serial.printf("   Found test at line %d (%s format)\n", j, isNewFormat ? "NEW" : "OLD");

            // -------- Test fields --------
            String fileName = "", animalNo = "", deviceNo = "";
            String testNo = "", timeStamp = "";
            float EC[4] = {0, 0, 0, 0};
            String clinicalLine = "", subClinicalLine = "";
            String positivesLine = "";

            for (int k = j; k < j + 15 && k < lineCount; k++) {

                // ===== NEW FORMAT =====
                if (lines[k].startsWith("File Name:")) {
                    fileName = lines[k].substring(11);
                    fileName.trim();
                }
                else if (lines[k].startsWith("RFID No.:  ")) {
                    animalNo = lines[k].substring(10);
                    animalNo. trim();
                }
                else if (lines[k].startsWith("Hardware No.: ")) {
                    deviceNo = lines[k].substring(14);
                    deviceNo.trim();
                }
                else if (lines[k].startsWith("Test Count:")) {
                    testNo = lines[k].substring(12);
                    testNo.trim();
                }
                else if (lines[k].startsWith("Time: ")) {
                    timeStamp = lines[k].substring(6);
                    timeStamp. trim();
                }
                else if (lines[k].startsWith("EC: ")) {
                    int idx = 0;
                    int start = 4;
                    while (idx < 4 && start < lines[k].length()) {
                        int comma = lines[k].indexOf(',', start);
                        if (comma == -1) comma = lines[k].length();
                        EC[idx++] = lines[k].substring(start, comma).toFloat();
                        start = comma + 2;
                    }
                }
                else if (lines[k].startsWith("Clinical:")) {
                    clinicalLine = lines[k];
                }
                else if (lines[k].startsWith("Sub Clinical:")) {
                    subClinicalLine = lines[k];
                }
                
                // ===== OLD FORMAT =====
                else if (lines[k].startsWith("FileName")) {
                    fileName = lines[k].substring(9);
                    fileName.trim();
                }
                else if (lines[k].startsWith("RFIDno")) {
                    animalNo = lines[k]. substring(7);
                    animalNo.trim();
                }
                else if (lines[k].startsWith("HardwareNo")) {
                    deviceNo = lines[k].substring(11);
                    deviceNo.trim();
                }
                else if (lines[k].startsWith("TestCount")) {
                    testNo = lines[k].substring(10);
                    testNo.trim();
                }
                else if (lines[k].startsWith("time")) {
                    timeStamp = lines[k].substring(5);
                    timeStamp. trim();
                }
                else if (lines[k].startsWith("ECwComp")) {
                    int idx = 0;
                    int start = 8;
                    while (idx < 4 && start < lines[k]. length()) {
                        int comma = lines[k].indexOf(',', start);
                        if (comma == -1) comma = lines[k].length();
                        EC[idx++] = lines[k].substring(start, comma).toFloat();
                        start = comma + 2;
                    }
                }
                else if (lines[k].startsWith("Postives")) {
                    positivesLine = lines[k];
                }
            }

            // -------- Determine quarter results --------
            String printResultFL = "Normal";
            String printResultFR = "Normal";
            String printResultBL = "Normal";
            String printResultBR = "Normal";

            // NEW FORMAT (Clinical/Sub Clinical)
            if (clinicalLine.length() > 0 || subClinicalLine.length() > 0) {
                if (clinicalLine.indexOf("FL") != -1) printResultFL = "Clinical";
                else if (subClinicalLine.indexOf("FL") != -1) printResultFL = "Subclinical";

                if (clinicalLine.indexOf("FR") != -1) printResultFR = "Clinical";
                else if (subClinicalLine.indexOf("FR") != -1) printResultFR = "Subclinical";

                if (clinicalLine.indexOf("BL") != -1) printResultBL = "Clinical";
                else if (subClinicalLine.indexOf("BL") != -1) printResultBL = "Subclinical";

                if (clinicalLine.indexOf("BR") != -1) printResultBR = "Clinical";
                else if (subClinicalLine. indexOf("BR") != -1) printResultBR = "Subclinical";
            }
            // OLD FORMAT (Postives {clinical}{subclinical})
            else if (positivesLine.length() > 0) {
                int start = positivesLine.indexOf("{") + 1;
                int mid = positivesLine.indexOf("}{");
                int end = positivesLine.indexOf("}", mid + 2);

                String posBracket1 = "";
                String posBracket2 = "";

                if (start > 0 && mid > start && end > mid) {
                    posBracket1 = positivesLine.substring(start, mid);
                    posBracket2 = positivesLine.substring(mid + 2, end);
                    posBracket1.trim();
                    posBracket2.trim();
                }

                if (posBracket1.indexOf("FL") != -1) printResultFL = "Clinical";
                else if (posBracket2.indexOf("FL") != -1) printResultFL = "Subclinical";

                if (posBracket1.indexOf("FR") != -1) printResultFR = "Clinical";
                else if (posBracket2.indexOf("FR") != -1) printResultFR = "Subclinical";

                if (posBracket1.indexOf("BL") != -1) printResultBL = "Clinical";
                else if (posBracket2.indexOf("BL") != -1) printResultBL = "Subclinical";

                if (posBracket1.indexOf("BR") != -1) printResultBR = "Clinical";
                else if (posBracket2.indexOf("BR") != -1) printResultBR = "Subclinical";
            }

            // -------- Date from filename --------
            String dateStr = formatDateFromFilename(fileName. length() > 0 ? fileName 
                                                                          : fileList[i].filename);

            // -------- Format output --------
            String testOutput = "";
            testOutput += "Date: " + dateStr + "\n";
            testOutput += "Time: " + timeStamp + "\n";
            testOutput += "Animal No: " + animalNo + "\n";
            testOutput += "Device No: " + deviceNo + "\n";
            testOutput += "Test No: " + testNo + "\n";
            testOutput += "-----------------------------\n";
            testOutput += "FL: " + String(EC[0], 2) + "  " + printResultFL + "\n";
            testOutput += "FR:  " + String(EC[1], 2) + "  " + printResultFR + "\n";
            testOutput += "BL: " + String(EC[2], 2) + "  " + printResultBL + "\n";
            testOutput += "BR: " + String(EC[3], 2) + "  " + printResultBR + "\n";
            testOutput += "-----------------------------\n";

            testsToPrint[testsFound] = testOutput;
            testsFound++;
            
            Serial.printf("✅ Collected test #%d from %s (Test No: %s)\n", 
                         testsFound, fileList[i].filename.c_str(), testNo.c_str());
        }
        
        Serial.printf("Finished file %s - Total collected:  %d/%d\n", 
                     fileList[i]. filename.c_str(), testsFound, testsNeeded);
    }

    if (testsFound == 0) {
        Serial.println("⚠️ No test entries found in SPIFFS.");
        lastPrintHadTests = false;
        return;
    }

    lastPrintHadTests = true;

    // -------- BLE Print --------
    if (connected) {
        Serial.println("\n=== 🖨️ SENDING TO BLE PRINTER ===");
        
        SerialBT.println("       Last 5 Tests");
        SerialBT.println();
        
        for (int i = 0; i < testsFound; i++) {
            Serial.println("=== TEST " + String(i + 1) + " ===");
            Serial.println(testsToPrint[i]);
            
            SerialBT.println(testsToPrint[i]);
        }
        
        SerialBT.println("*** Printing complete!");
        SerialBT.println();
        SerialBT.println();
        
        Serial.printf("✅ Printed %d tests successfully\n", testsFound);
    } else {
        Serial.println("⚠️ BLE Printer not connected!");
    }
}
void print5setup() {
    if (! SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed!  Formatting SPIFFS...");
        SPIFFS.format();
        if (!SPIFFS.begin(true)) {
            Serial.println("SPIFFS Remount Failed!");
            return;
        }
    }

    listDir(SPIFFS, "/", 0);
    bool spiffsWasEmptyAtStart = (fileCount == 0);

    sortFilesByDate();

    Serial.println("\n=== MOST RECENT FILES (AFTER SORT) ===");
    int filesToPrint = (fileCount < 5) ? fileCount : 5;
    if (filesToPrint == 0) {
        lastPrintHadTests = false;
        return;
    }
    for (int i = 0; i < filesToPrint; i++) {
        Serial.printf("Recent:  %s\n", fileList[i].filename.c_str());
    }

    checkSPIFFSStorage();

    bleSetup();
}

void tpPrint(ElectricalConductivity elCdvty) {
    tft.drawText(40, 80, "Searching", COLOR_YELLOW);
    tft.drawText(60, 110, "for", COLOR_YELLOW);
    tft.drawText(10, 140, "printer nearby", COLOR_YELLOW);

    bool connected = tpsetup();
    if (connected) {
        tft.clear();
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
        delay(100);
        tft.drawText(30, 110, "CONNECTED! !", COLOR_GREEN);
        delay(2000);
        tft.clear();
        tft.drawText(30, 100, "PRINTING...");
        SerialBT.begin();

        SerialBT.println("         CHIMERTECH");

        SerialBT.println("         Quadmastest");
        SerialBT.println("Date     :  " + String(currentDate));
        SerialBT.println("Time     : " + String(currentTime));
        SerialBT.println("Animal No: " + String(rfidNumber));
        SerialBT.println("Device No: " + String(uniqueChipID));
        SerialBT.println("Test No  : " + String(testsCounter));
        SerialBT.println("--------------------------------");
        SerialBT.println("FL :  " + String(elCdvty.ecWithComp[0]) + "   " + resultFL);
        SerialBT.println("FR : " + String(elCdvty.ecWithComp[1]) + "   " + resultFR);
        SerialBT.println("BL : " + String(elCdvty.ecWithComp[2]) + "   " + resultBL);
        SerialBT.println("BR :  " + String(elCdvty.ecWithComp[3]) + "   " + resultBR);
        SerialBT.println();
        SerialBT.println("----------------------------");

        SerialBT.println("CONTACT US AT:");
        SerialBT.println("research@chimertech.com");
        SerialBT.println("+91 9790929442");
        SerialBT.println("www.chimertech.com");
        SerialBT.println();
        SerialBT.println();

        tft.clear();
        tft.drawText(20, 100, "PRINTING DONE! !");
        delay(2000);
        tft.clear();
        SerialBT.disconnect();
    } else {
        tft. clear();
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
        delay(100);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
        delay(100);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
        delay(100);
        tft.drawText(40, 80, "COULD NOT", COLOR_RED);
        tft.drawText(60, 110, "FIND", COLOR_RED);
        tft.drawText(40, 140, "PRINTER", COLOR_RED);
        delay(5000);
    }
    return;
}

void tpPrint2() {
    bool connected = tpsetup();
    if (connected) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
        delay(100);
        SerialBT.begin();
        SerialBT.println("        CHIMERTECH");
        SerialBT.println("           NIRAAM");
        SerialBT.println("Date:  02/07/25 ");
        SerialBT.println("Animal No: COW23045 ");
        SerialBT.println("Device No: ");
        SerialBT.println("Test No: ");
        SerialBT.println("--------------------------------");
        SerialBT.println("Milk Fat %: 3.4");
        SerialBT.println("Crude Protein %: 0.59");
        SerialBT.println("True Protein %: 2.53 ");
        SerialBT.println("Lactose %: 4.86");
        SerialBT.println("Fatty Acids:  ");
        SerialBT.println("MUN (mg/dL): 16");
        SerialBT.println("SNF %: 9.70 ");
        SerialBT.println("Casein:  2.6");
        SerialBT.println("Citric Acid:");
        SerialBT.println("TMS:  12.08");
        SerialBT.println("PAG: 0");
        SerialBT.println("Estrone sulfate: 0");
        SerialBT.println("Progesterone: 0");
        SerialBT.println("Total protein: 3.13");
        SerialBT.println("Water:  87.92");
        SerialBT.println("Penicillin: 0");
        SerialBT.println("Tetracycline: 0");
        SerialBT.println("Cortisol: 0.69");
        SerialBT.println("BHB: 0");
        SerialBT.println(" ");
        SerialBT.println("SCC: 90000");
        SerialBT.println("Electrical Conductivity:");
        SerialBT.println(" FL: 5.2");
        SerialBT.println(" FR: 5.2");
        SerialBT.println(" BL: 5.3");
        SerialBT.println(" BR: 5.2");
        SerialBT.println("Temperature");
        SerialBT.println(" FL: 37.2");
        SerialBT.println(" FR: 37.1");
        SerialBT.println(" BL: 37.2");
        SerialBT.println(" BR: 37.3");
        SerialBT.println("pH:");
        SerialBT.println(" FL: 6.5");
        SerialBT.println(" FR: 6.5");
        SerialBT.println(" BL: 6.5");
        SerialBT.println(" BR: 6.5");
        SerialBT.println("Milk Yield: 8");
        SerialBT.println("----------------------------");
        SerialBT.println("CONTACT US AT:");
        SerialBT.println("research@chimertech.com");
        SerialBT.println("9790929442");
        SerialBT.println();
        SerialBT.println();
    } else {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
        delay(100);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
        delay(100);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
        delay(100);
        delay(5000);
    }
    return;
}

void testPrintLast5Tests() {
    Serial.println("\n=== TESTING printLast5Tests FUNCTION ===");

    struct TestFile {
        String filename;
        String content;
    };

    TestFile testFiles[] = {
        {"latestFile.txt", "FileName latestFile.txt\nTest1\nTest2\nTest3\nTest4\nTest5\n"},
        {"previousFile1.txt", "FileName previousFile1.txt\nTest6\nTest7\n"},
        {"previousFile2.txt", "FileName previousFile2.txt\nTest8\nTest9\nTest10\nTest11\nTest12\n"},
        {"emptyFile.txt", ""}};

    int numTestFiles = sizeof(testFiles) / sizeof(TestFile);

    for (int i = 0; i < numTestFiles; i++) {
        Serial.printf("Loading file: %s\n", testFiles[i].filename.c_str());
        SPIFFS.remove(testFiles[i].filename);
        File file = SPIFFS.open("/" + testFiles[i].filename, FILE_WRITE);
        if (file) {
            file.print(testFiles[i].content);
            file.close();
            Serial.printf("File %s created successfully.\n", testFiles[i].filename.c_str());
        } else {
            Serial.printf("❌ Failed to create file: %s\n", testFiles[i].filename.c_str());
        }
    }

    Serial.println("\nRunning printLast5Tests() to validate functionality...");
    printLast5Tests();
}