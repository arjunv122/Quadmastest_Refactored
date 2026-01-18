#include "Calibration.h"
#include "Sensors.h"
extern int EEPROMAddressACal;      // ← ADD THIS
extern bool solAFlag;               // ← ADD THIS
extern float temperatureLow[4];    // ← ADD THIS

void autoCalBegin() {
    int KVALUEADDR = EEPROMAddressACal;
    for (int i = 0; i < 4; i++) {
        kvalueLow[i] = EEPROM.readFloat(KVALUEADDR);
        Serial.println(F("kvalueLow read at Setup"));
        Serial.print(String(kvalueLow[i]));
        Serial.println(F(" at address:  "));
        Serial.print(String(KVALUEADDR));
        
        if (EEPROM.readFloat(KVALUEADDR) == 0xFF && EEPROM.readFloat(KVALUEADDR + 1) == 0xFF && 
            EEPROM.readFloat(KVALUEADDR + 2) == 0xFF && EEPROM.readFloat(KVALUEADDR + 3) == 0xFF) {
            kvalueLow[i] = 1.0;
            EEPROM.writeFloat(KVALUEADDR, kvalueLow[i]);
            EEPROM.commit();
        }
        
        kvalueHigh[i] = EEPROM.readFloat(KVALUEADDR + 16);
        Serial.println(F("kvalueHigh read at setup"));
        Serial.print(String(kvalueHigh[i]));
        Serial.print(F(" at address: "));
        Serial.print(String(KVALUEADDR + 16));
        
        if (EEPROM.readFloat(KVALUEADDR + 16) == 0xFF && EEPROM. readFloat(KVALUEADDR + 17) == 0xFF && 
            EEPROM. readFloat(KVALUEADDR + 18) == 0xFF && EEPROM.readFloat(KVALUEADDR + 19) == 0xFF) {
            kvalueHigh[i] = 1.0;
            EEPROM.writeFloat(KVALUEADDR + 4, kvalueHigh[i]);
            EEPROM.commit();
        }
        kvalue[i] = kvalueLow[i];
        KVALUEADDR += sizeof(kvalue[i]);
    }
}

struct ElectricalConductivity AutoCalibrationRoutine(struct ElectricalConductivity elCdvty, float temperature[4]) {
    static boolean ecCalibrationFinish = 0;
    static boolean enterCalibrationFlag = 0;
    static float compECsolution;
    float ecWithComp[4];
    int KVALUEADDR = EEPROMAddressACal;
    float value[4] = {0.0}, valueTemp[4] = {0.0};
    float KValueTemp[4];

    for (int i = 0; i < 4; i++) {
        if ((elCdvty.ecWithoutComp[i] > 2.00) && (elCdvty.ecWithoutComp[i] < 6.00))
            ecWithComp[i] = 4.00 * (1.0 + 0.019 * (temperature[i] - 25.0));
        else if ((elCdvty.ecWithoutComp[i] > 6.00) && (elCdvty.ecWithoutComp[i] < 12.00)) {
            ecWithComp[i] = 8.00 * (1.0 + 0.019 * (temperature[i] - 25.0));
        } else {
            Serial.println(F("EC values are not in range required for Auto Cal"));
            Serial.print(String(elCdvty. ecWithoutComp[i]));
        }

        KValueTemp[i] = ecWithComp[i] / elCdvty.ecWithoutComp[i];
        Serial.println(F("ecWithComp:  "));
        Serial.print(String(ecWithComp[i]));
        Serial.println(F("KValueTemp value is: "));
        Serial.println(String(KValueTemp[i]));
        Serial.println(F(". "));
        
        if ((KValueTemp[i] > 0.5) && (KValueTemp[i] < 1.5)) {
            if ((elCdvty.ecWithoutComp[i] > 2.00) && (elCdvty.ecWithoutComp[i] < 6.00)) {
                kvalueLow[i] = KValueTemp[i];
            } else if ((elCdvty.ecWithoutComp[i] > 6.00) && (elCdvty.ecWithoutComp[i] < 12.00)) {
                kvalueHigh[i] = KValueTemp[i];
            }
            ecCalibrationFinish = 1;
            Serial.println(F("SUCCESS"));
        } else {
            ecCalibrationFinish = 0;
            Serial.println(F("NOT SUCCESS"));
        }
    }

    for (int i = 0; i < 4; i++) {
        Serial.println(F("kvalueLow: "));
        Serial.print(String(kvalueLow[i]));
        Serial.println(F(". "));
    }

    for (int i = 0; i < 4; i++) {
        Serial.println(F("kvalueHigh:  "));
        Serial.print(String(kvalueHigh[i]));
        Serial.println(F("."));
    }

    for (int i = 0; i < 4; i++) {
        if (ecCalibrationFinish) {
            if ((elCdvty.ecWithoutComp[i] > 2.00) && (elCdvty.ecWithoutComp[i] < 6.00)) {
                EEPROM.writeFloat(KVALUEADDR, kvalueLow[i]);
                Serial.println(F("kvalueLow written:  "));
                Serial.print(String(kvalueLow[i]));
                Serial.print(F("at address"));
                Serial.print(String(KVALUEADDR));
                EEPROM.commit();
                float tempVal = EEPROM.readFloat(KVALUEADDR);
                Serial.println(F("value read for kvalueLow: "));
                Serial.print(String(tempVal));
                calibrationA = true;
            } else if ((elCdvty.ecWithoutComp[i] > 6.00) && (elCdvty. ecWithoutComp[i] < 12.00)) {
                EEPROM.writeFloat(KVALUEADDR + 16, kvalueHigh[i]);
                Serial.println(F("kvalueHigh written:  "));
                Serial.print(String(kvalueHigh[i]));
                Serial.print(F("at address"));
                Serial.print(String(KVALUEADDR + 16));
                EEPROM.commit();
                float tempVal = EEPROM.readFloat(KVALUEADDR + 16);
                Serial.println(F("value read for kvalueHigh: "));
                Serial.print(String(tempVal));
                calibrationB = true;
            }
            Serial.println(F(">>>Calibration Successful"));
        } else {
            Serial.println(F(">>>Calibration Failed"));
        }
        KVALUEADDR += sizeof(kvalue[i]);
    }

    elCdvty = calibrateECValues(elCdvty, temperatures);

    elCdvty. ecWithComp[0] = ((elCdvty.ecWithoutComp[0] / (1 + (0.019 * (temperature[0] - 25)))));
    elCdvty.ecWithComp[1] = ((elCdvty. ecWithoutComp[1] / (1 + (0.019 * (temperature[1] - 25)))));
    elCdvty.ecWithComp[2] = ((elCdvty.ecWithoutComp[2] / (1 + (0.019 * (temperature[2] - 25)))));
    elCdvty.ecWithComp[3] = ((elCdvty. ecWithoutComp[3] / (1 + (0.019 * (temperature[3] - 25)))));

    Serial.println(F("EC0 w cal & T corr"));
    Serial.print(String(elCdvty.ecWithComp[0]));
    Serial.println(F("EC1 w cal & T corr "));
    Serial.print(String(elCdvty.ecWithComp[1]));
    Serial.println(F("EC2 w cal & T corr "));
    Serial.print(String(elCdvty.ecWithComp[2]));
    Serial.println(F("EC3 w cal & T corr "));
    Serial.print(String(elCdvty. ecWithComp[3]));
    return elCdvty;
}

struct ElectricalConductivity calibrateECValues(struct ElectricalConductivity elCdvty, float temperature[4]) {
    float value[4] = {0.0}, valueTemp[4] = {0.0};
    for (int i = 0; i < 4; i++) {
        valueTemp[i] = elCdvty.ecWithoutComp[i] * kvalue[i];
        if (valueTemp[i] > 6) {
            kvalue[i] = kvalueHigh[i];
        } else if (valueTemp[i] < 6) {
            kvalue[i] = kvalueLow[i];
        }

        value[i] = elCdvty.ecWithoutComp[i] * kvalue[i];
        elCdvty.ecWithoutComp[i] = value[i];
    }
    Serial.println(F("EC 0 post cal"));
    Serial.print(String(elCdvty.ecWithoutComp[0]));
    Serial.println(F("EC 1 post cal"));
    Serial.print(String(elCdvty.ecWithoutComp[1]));
    Serial.println(F("EC 2 post cal"));
    Serial.print(String(elCdvty.ecWithoutComp[2]));
    Serial.println(F("EC 3 post cal"));
    Serial.print(String(elCdvty.ecWithoutComp[3]));
    return elCdvty;
}

void skipForACSol() {
    for (int i = 60; i > 0; i--) {
        tft.setFont(Terminal12x16);
        tft.drawText(31, 80, "Please pour ", COLOR_YELLOW);
        tft.drawText(36, 105, "Solution", COLOR_GREEN);
        tft.drawText(21, 130, "till cup mark ", COLOR_YELLOW);
        tft.drawText(2, 186, "Skip Time- " + String(i) + "Sec", COLOR_GOLD);
        delay(500);

        tft.setFont(Terminal6x8);
        tft.drawText(5, 207, "(Skip - Press Next Button)", COLOR_GREEN);
        delay(500);
        analogReadResolution(10);
        analogSetWidth(10);
        int buttonState = 0;
        buttonState = digitalRead(NEXT_PIN);
        if (buttonState == HIGH) {
            solAFlag = true;
            return;
        }
    }
    return;
}

struct ElectricalConductivity renderManCal(struct ElectricalConductivity elCdvty, float temperatures[4]) {
    char buffer[32];
    bool calibrationDone = false;

    while (!calibrationDone) {
        tft.clear();
        float *temperature = measureTemp(temperatures);
        elCdvty = measureECWithoutComp(temperatures, elCdvty);

        tft.drawText(20, 20, "Manual EC Calibration");

        for (int i = 0; i < 4; i++) {
            sprintf(buffer, "C%d T:  %. 2f ", i, temperature[i]);
            tft.drawText(20, 40 + (i * 40), buffer);

            sprintf(buffer, "C%d EC: %.2f", i, elCdvty. ecWithoutComp[i]);
            tft.drawText(20, 55 + (i * 40), buffer);
        }

        for (int i = 0; i < 4; i++) {
            Serial.print("Channel ");
            Serial.print(i);
            Serial.print(" Temp: ");
            Serial.print(temperature[i], 2);
            Serial.print(" C, EC (no comp): ");
            Serial. print(elCdvty. ecWithoutComp[i], 3);
            Serial.println(" mS/cm");
        }

        delay(100);
    }

    return elCdvty;
}