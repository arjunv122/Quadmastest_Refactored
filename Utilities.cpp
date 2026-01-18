#include "Utilities.h"

uint32_t getChipId() {
    for (int i = 0; i < 17; i = i + 8) {
        uniqueChipID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    Serial.print("ESP32 Chip ID is: " + String(uniqueChipID));
    return uniqueChipID;
}

void resetTestCounter() {
    testsCounter = 0;
    EEPROM.writeInt(0, 0);
    EEPROM.commit();
    Serial.println(F("Test counter RESET to 0"));
}

void clearSPIFFS() {
    Serial.println("\n⚠️ CLEAR SPIFFS:  STARTING FORMAT.. .");

    if (! SPIFFS.begin(true)) {
        Serial.println("❌ SPIFFS mount failed.  Trying forced format...");
        SPIFFS.format();
        if (!SPIFFS.begin(true)) {
            Serial.println("❌ SPIFFS re-mount failed even after format!");
            return;
        }
    }

    Serial.println("🧹 Formatting SPIFFS...");

    bool result = SPIFFS.format();
    if (result) {
        Serial.println("✅ SPIFFS successfully formatted!");
    } else {
        Serial.println("❌ SPIFFS format failed!");
    }

    if (! SPIFFS.begin(true)) {
        Serial.println("❌ ERROR: SPIFFS could not be remounted after format.");
    } else {
        Serial.println("✅ SPIFFS remounted, ready for use.");
    }
}