#include "RFID.h"
#include "logo.h"  // ← ADD THIS

extern byte nuidPICC[4];      // ← ADD THIS
extern String tagContent;     // ← ADD THIS

void readRFID() {
    tft.clear();
    tft.setFont(Terminal12x16);
    tft.drawBitmap(5, 5, COW, 150, 130, COLOR_LIGHTBLUE);
    tft.drawBitmap(115, 105, RFID, 50, 30, COLOR_BLUE);
    tft.drawText(65, 140, "RFID", COLOR_GREEN);
    int flag = 0;

    for (int i = 30; i > 0; i--) {
        if (rfid. PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
            pinMode(BUZZER_PIN, OUTPUT);
            digitalWrite(BUZZER_PIN, HIGH);
            delay(100);
            digitalWrite(BUZZER_PIN, LOW);
            delay(100);
            Serial.println("Card has been read!");
            flag = 1;
            break;
        } else {
            analogReadResolution(10);
            analogSetWidth(10);
            tft.drawBitmap(115, 105, RFID, 50, 30, COLOR_LIGHTBLUE);
            tft.setFont(Terminal12x16);
            tft.drawText(2, 186, "Skip Time- " + String(i) + "Sec", COLOR_GOLD);
            delay(500);
            tft.drawBitmap(115, 105, RFID, 50, 30, COLOR_BLUE);
            tft.setFont(Terminal6x8);
            tft.drawText(5, 207, "(Skip - Press Next Button)", COLOR_GREEN);
            delay(500);
            int buttonState = 0;
            buttonState = digitalRead(NEXT_PIN);
            if (buttonState == HIGH)
                return;
        }
    }

    if (flag == 0)
        return;

    if (! rfid.PICC_IsNewCardPresent()) {
    }

    if (!rfid.PICC_ReadCardSerial()) {
    }

    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = rfid. PICC_GetType(rfid.uid.sak);
    Serial.println(rfid.PICC_GetTypeName(piccType));

    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("Your tag is not of type MIFARE Classic."));
    }

    if (rfid.uid.uidByte[0] != nuidPICC[0] ||
        rfid.uid.uidByte[1] != nuidPICC[1] ||
        rfid.uid.uidByte[2] != nuidPICC[2] ||
        rfid.uid.uidByte[3] != nuidPICC[3]) {
        Serial.println(F("A new card has been detected."));

        for (byte i = 0; i < 4; i++) {
            nuidPICC[i] = rfid.uid.uidByte[i];
        }

        Serial.println(F("The NUID tag is: "));
        Serial.print(F("In hex: "));
        Serial.println();
        Serial.print(F("In dec: "));
        tagContent = printDec(rfid.uid.uidByte, rfid.uid.size);
        Serial.println(tagContent);
    } else {
        Serial.println(F("Card read previously."));
        Serial.println(F("The NUID tag is:"));
        Serial.print(F("In hex: "));
        Serial.println();
        Serial.print(F("In dec: "));
        tagContent = printDec(rfid.uid.uidByte, rfid.uid.size);
    }
    
    tft.clear();
    tft.setFont(Terminal11x16);
    tft.drawBitmap(5, 5, COW, 150, 130, COLOR_LIGHTBLUE);
    tft.drawBitmap(115, 105, RFID, 50, 30, COLOR_BLUE);
    tft.drawText(65, 140, "RFID", COLOR_GREEN);
    tft.drawText(20, 163, String(tagContent), COLOR_GREEN);
    delay(5000);

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}

String printDec(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < rfid.uid.size&&i<7; i++) {
        rfidNumber.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
        rfidNumber.concat(String(rfid.uid.uidByte[i], HEX));
    }

    rfidNumber.toUpperCase();
    return rfidNumber;
}