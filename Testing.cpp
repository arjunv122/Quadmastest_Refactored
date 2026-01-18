#include "Testing.h"
#include "Sensors.h"
#include "Display.h"
#include "Calibration.h"
#include "FileSystem.h"
#include "Printing.h"

struct ElectricalConductivity renderQuadmastestRoutine(struct ElectricalConductivity elCdvty, float temperatures[4]) {
    tft.clear();
    float *temperature;
    int analogVal1, analogVal2, analogVal3, analogVal4;
    int num = 1;

    tft.setFont(Terminal12x16);
    tft.drawText(25, 85, "Analyzing.. .");
    tft.drawText(27, 115, "Wait 3 Sec !");
    temperature = measureTemp(temperatures);
    elCdvty = measureECWithoutComp(temperatures, elCdvty);
    delay(3000);

    // Average values for temperature
    temperature[0] = temperature[0] / num;
    temperature[1] = temperature[1] / num;
    temperature[2] = temperature[2] / num;
    temperature[3] = temperature[3] / num;

    // Average values for elCdvty without compensation
    elCdvty.ecWithoutComp[0] = elCdvty. ecWithoutComp[0] / num;
    elCdvty.ecWithoutComp[1] = elCdvty.ecWithoutComp[1] / num;
    elCdvty.ecWithoutComp[2] = elCdvty.ecWithoutComp[2] / num;
    elCdvty.ecWithoutComp[3] = elCdvty.ecWithoutComp[3] / num;

    temperatures = temperature;

    Serial.println("Average EC w/o 0 - " + String(elCdvty.ecWithoutComp[0]));
    Serial.println("Average EC w/o 1 - " + String(elCdvty.ecWithoutComp[1]));
    Serial.println("Average EC w/o 2 - " + String(elCdvty.ecWithoutComp[2]));
    Serial.println("Average EC w/o 3 - " + String(elCdvty.ecWithoutComp[3]));

    if (autoCalFlag == false) {
        elCdvty = calibrateECValues(elCdvty, temperatures);
    }

    Serial.println("Calibrated EC 0 - " + String(elCdvty.ecWithComp[0]));
    Serial.println("Calibrated EC 1 - " + String(elCdvty.ecWithComp[1]));
    Serial.println("Calibrated EC 2 - " + String(elCdvty.ecWithComp[2]));
    Serial.println("Calibrated EC 3 - " + String(elCdvty.ecWithComp[3]));

    elCdvty. ecWithComp[0] = ((elCdvty.ecWithoutComp[0] / (1 + (0.019 * (temperature[0] - 25)))));
    elCdvty.ecWithComp[1] = ((elCdvty.ecWithoutComp[1] / (1 + (0.019 * (temperature[1] - 25)))));
    elCdvty. ecWithComp[2] = ((elCdvty.ecWithoutComp[2] / (1 + (0.019 * (temperature[2] - 25)))));
    elCdvty.ecWithComp[3] = ((elCdvty.ecWithoutComp[3] / (1 + (0.019 * (temperature[3] - 25)))));
    
    tft.clear();
    renderBatteryLevelAndQuadPie();
    displayTemp(temperature);
    delay(3000);
    tft.clear();
    renderBatteryLevelAndQuadPie();
    invalidInput = renderRecheckInputMessage(elCdvty);
    displayEC(elCdvty, invalidInput);

    Serial.println("EMPTY CHAMBERS IN THE FUNCTION BEFORE TEST COUNT" + String(invalidInput));
    
    if (invalidInput >= 3) {
        Serial.println("buzzer should ring here");
        tft.clear();
        tft.drawText(30, 105, "INVALID TEST", COLOR_RED);
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
        return elCdvty;
    } else if (autoCalFlag) {
        return elCdvty;
    } else {
        testsCounter = EEPROM.readInt(0);
        Serial.println("test count read from address" + String(testsCounter));
        testsCounter = testsCounter + 1;
        Serial.println("test count after incrementation" + String(testsCounter));
        EEPROM.writeInt(0, testsCounter);
        EEPROM.commit();
        testsCounter = EEPROM. readInt(0);
        Serial.println("test count after reading again" + String(testsCounter));
        Serial.println("SensorCounter - " + String(testsCounter));
        renderSensorCounter();
        prepareFilesForWrite();
        fileWrite(elCdvty, temperatures, uniqueChipID);
        delay(5000);
        tft.clear();
        
        for (int i = 10; i > 0; i--) {
            int buttonState = digitalRead(NEXT_PIN);
            if (buttonState == HIGH) {
                tft.clear();
                Serial.println("Print initiated!");
                Serial.println("Scanning will start");
                tpPrint(elCdvty);
                break;
            } else {
                tft.setFont(Terminal11x16);
                tft.drawText(40, 70, "Press NEXT", COLOR_WHITE);
                tft.drawText(50, 100, "in " + String(i) + " sec", COLOR_GOLD);
                tft.drawText(35, 130, "to scan for");
                tft.drawText(12, 160, "THERMAL PRINTER", COLOR_RED);
                delay(1000);
            }
        }

        return elCdvty;
    }
    return elCdvty;
}