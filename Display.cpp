#include "Display.h"
#include "Sensors.h"
#include "Testing.h"
#include "Calibration.h"
#include "RFID.h"
#include "Printing.h"
#include "Utilities.h"
#include "TimeManager.h"
#include "wifiUpdateAndSPIFFSDirectory.h"
extern float temperatureLow[4];  // ← ADD THIS
void renderLogo() {
    Serial.println(F("Welcome sequence started... "));
    int chipId = getChipId();
    tft.setOrientation(0);
    tft.setFont(Terminal12x16);
    tft.drawText(25, 35, "QUADMASTEST", COLOR_TOMATO);
    
    for (uint8_t i = 0; i < 50; i++) {
        tft.drawPixel(random(tft.maxX()), random(tft.maxY()), random(0xffff));
    }

    tft.drawBitmap(15, 75, Logo, 150, 80, COLOR_GREEN);
    tft.setFont(Terminal6x8);
    tft.drawText(18, 175, "Chimertech Private Ltd", COLOR_GREEN);
    tft.drawText(20, 190, "Version - " + String(VersionString));
    tft.drawText(5, 205, "Device Chip Id - " + String(chipId));
}

void renderWelcomePage() {
    int chipId = getChipId();

    // 1) Battery gate
    if (checkBatteryAndRenderPrompt()) {
        Serial.println(F("SWITCHING OFF"));
        tft.clear();
        routineCompleteFlag = true;
        return;
    }

    int ACpressed   = 0;
    int prevPressed = 0;
    int nextPressed = 0;

    // 2) Sample buttons for up to 5 seconds
    for (int i = 5; i > 0; i--) {
        ACpressed   = digitalRead(AUTOCAL_PIN);
        prevPressed = digitalRead(PREVIOUS_PIN);
        nextPressed = digitalRead(NEXT_PIN);

        if (ACpressed == HIGH || prevPressed == HIGH || nextPressed == HIGH) {
            break;
        }
        delay(1000);
    }

    // Priority 1: AUTO CALIBRATION
    if (ACpressed == HIGH) {
        Serial.println("AC mode detection started...");
        int countdown = 5;
        bool cancelled = false;

        while (countdown > 0) {
            if (digitalRead(AUTOCAL_PIN) == LOW) {
                Serial.println("AC mode cancelled.");
                cancelled = true;
                break;
            }

            tft.clear();
            tft.setFont(Terminal12x16);
            tft.setBackgroundColor(COLOR_BLACK);
            tft.drawText(65, 80, "AUTO", COLOR_YELLOW);
            tft.drawText(25, 105, "CALIBRATION", COLOR_YELLOW);
            tft.drawText(45, 130, "MODE in " + String(countdown), COLOR_YELLOW);
            delay(1000);
            countdown--;
        }

        if (! cancelled && digitalRead(AUTOCAL_PIN) == HIGH) {
            Serial. println("AC mode activated!");
            autoCalFlag = true;
            tft.clear();
            renderAutoCalibrationPage();
            routineCompleteFlag = true;
            return;
        }
    }

    // Priority 2: WIFI SETUP (PREVIOUS)
    if (prevPressed == HIGH) {
        Serial.println("PREVIOUS press mode ---");
        tft.clear();
        tft.setFont(Terminal12x16);
        tft.drawText(50, 80, "Loading", COLOR_YELLOW);
        tft.drawText(30, 110, "Wifi Setup", COLOR_GREEN);
        tft.drawText(70, 140, "mode", COLOR_YELLOW);
        tft.setFont(Terminal6x8);
        tft.drawText(18, 175, "Chimertech Private Ltd", COLOR_GREEN);
        tft.drawText(20, 190, "Version - " + String(VersionString));
        tft.drawText(5, 205, "Device Chip Id - " + String(chipId));

        wifiFlag = true;
        String IPAddress = Wifisetup(uniqueChipID);

        tft.clear();
        tft.setFont(Terminal12x16);
        tft.drawText(10, 80, "Wifi Connected!", COLOR_WHITE);
        tft.drawText(10, 110, "IP:", COLOR_YELLOW);
        tft.drawText(10, 135, IPAddress, COLOR_GREEN);

        syncRTCFromNTP_UTC_once();
        logUTCandIST("After manual sync");

        DateTime istDisplay = DateTime(rtc.now().unixtime() + IST_OFFSET);
        tft.setFont(Terminal6x8);
        tft.drawText(10, 160, "Time (IST):", COLOR_YELLOW);
        tft.drawText(10, 172, String(istDisplay.timestamp()), COLOR_GREEN);

        routineCompleteFlag = true;
        return;
    }

    // Priority 3: PRINT LAST 5 TESTS (NEXT)
    if (nextPressed == HIGH) {
        Serial.println("PRINTING FIVE RECENT TESTS ---");
        tft.clear();
        tft.setFont(Terminal12x16);
        tft.drawText(50, 80, "Printing", COLOR_YELLOW);
        tft.drawText(55, 110, "last 5", COLOR_GREEN);
        tft.drawText(60, 140, "tests", COLOR_YELLOW);
        delay(1500);

        tft.clear();
        tft.drawText(40, 80, "Searching", COLOR_YELLOW);
        tft.drawText(80, 110, "for", COLOR_YELLOW);
        tft.drawText(10, 140, "printer nearby", COLOR_YELLOW);

        print5setup();
        
        if (! connected) {
            tft.clear();
            tft.drawText(15, 90, "PRINTER", COLOR_RED);
            tft.drawText(15, 120, "NOT FOUND", COLOR_RED);
            digitalWrite(BUZZER_PIN, HIGH);
            delay(100);
            digitalWrite(BUZZER_PIN, LOW);
            delay(100);
            digitalWrite(BUZZER_PIN, HIGH);
            delay(100);
            digitalWrite(BUZZER_PIN, LOW);
            routineCompleteFlag = true;
            return;
        }
        
        if (! lastPrintHadTests) {
            tft.clear();
            tft.drawText(40, 80, "NO", COLOR_YELLOW);
            tft.drawText(80, 110, "FILES", COLOR_YELLOW);
            tft.drawText(10, 140, "FOUND", COLOR_YELLOW);
            Serial.println("No files found for last 5 tests");
            routineCompleteFlag = true;
            return;
        }

        tft.clear();
        tft.drawText(30, 100, "PRINT DONE", COLOR_GREEN);
        delay(1500);

        routineCompleteFlag = true;
        return;
    }

    // Default: run a normal test
    Serial.println(F("RFID starts"));
    readRFID();
    elCdvty = renderQuadmastestRoutine(elCdvty, temperatures);
    routineCompleteFlag = true;
}

bool checkBatteryAndRenderPrompt() {
    int value = mcp.analogRead(BATTERY_PIN);
    Serial.println("Raw ADC Value 1: " + String(value));
    battPercentage = 100 * ((value - 455) / (double)(535 - 455));
    battPercentage = constrain(battPercentage, 0, 100);
    Serial.println("battery percent METHOD 2 " + String(battPercentage));
    
    bool switchOff = false;

    if (battPercentage < 10) {
        tft.clear();
        tft.setFont(Terminal11x16);
        tft.setBackgroundColor(COLOR_BLACK);
        tft.drawBitmap(65, 35, lowbatt, 60, 55, COLOR_RED);

        if (battPercentage < 5) {
            tft.drawText(10, 105, "BATTERY CRITICAL", COLOR_RED);
            tft.drawText(20, 135, "SWITCHING OFF", COLOR_RED);
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
            switchOff = true;
        } else {
            tft.drawText(35, 105, "LOW BATTERY", COLOR_RED);
            tft.drawText(10, 135, "CONNECT CHARGER", COLOR_RED);
            pinMode(BUZZER_PIN, OUTPUT);
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
    }

    return switchOff;
}

void renderBatteryLevelAndQuadPie() {
    // ---- TIME (read once, UTC->IST) ----
    DateTime utc = rtc.now();  // fresh read, RTC is UTC
    DateTime ist = DateTime(utc.unixtime() + IST_OFFSET);

    // ---- BACKGROUND / STATIC UI ----
    tft.drawCircle(88, 108, 86, COLOR_BLUE);  // quad circle
    tft.drawLine(88, 22, 88, 193, COLOR_BLUE);
    tft.drawLine(2, 108, 173, 108, COLOR_BLUE);

    tft.fillRectangle(0, 0, 174, 20, COLOR_DARKBLUE);  // top bar
    tft.setFont(Terminal6x8);
    tft.setBackgroundColor(COLOR_DARKBLUE);

    // Date DD/MM/YYYY
    tft.drawText(60, 3, two(ist.day()) + "/" + two(ist.month()) + "/" + String(ist.year()));

    // For filenames if you need
    currentDate = two(ist.day()) + "." + two(ist.month()) + "." + String(ist.year());
    Serial.println(F("Todays date is: "));
    Serial.println(currentDate);

    // Time (12h, zero-padded, AM/PM)
    tft.drawText(73, 13, fmt12h(ist));

    // ---- BATTERY ----
    int raw = mcp.analogRead(BATTERY_PIN);
    bool isCharging = (digitalRead(CHARGER_VOLT) == HIGH);  // ✅ ADD THIS
    Serial.println("Raw ADC:  " + String(raw));

    // Single computation, clamped 0–100
    double frac = (double)(raw - BAT_MIN_RAW) / (double)(BAT_MAX_RAW - BAT_MIN_RAW);
    int battPct = (int)round(frac * 100.0);
    battPct = constrain(battPct, 0, 100);
    
    battPercentage = battPct;  // ✅ UPDATE GLOBAL VARIABLE

    Serial.println("Battery %: " + String(battPct));

    // Icon + color by level
    uint16_t col = (battPct >= 60) ? COLOR_GREEN : (battPct >= 20) ? COLOR_ORANGE : COLOR_RED;

    // ✅ ADD CHARGING DETECTION
    if (isCharging) {
        tft.drawBitmap(143, 3, Batt, 30, 15, COLOR_BLUE);  // Indicate charging state
        tft.drawText(146, 7, "CHG", COLOR_BLUE);
    } else {
        tft. drawBitmap(143, 3, Batt, 30, 15, col);
        tft.drawText(146, 7, String(battPct) + "%", col);
    }
}
int renderRecheckInputMessage(struct ElectricalConductivity elCdvty) {
    Serial.println("checking if input is empty or not");
    String recheckMessage = "";
    const String chambers[4] = {"FL", "FR", "BL", "BR"};
    
    if (elCdvty.ecWithComp[0] < 0.5) {
        recheckMessage = recheckMessage + chambers[0];
        invalidInput++;
        invalid[0] = true;
    }
    if (elCdvty.ecWithComp[1] < 0.5) {
        recheckMessage = recheckMessage + (recheckMessage == NULL ? "" : ",") + chambers[1];
        invalidInput++;
        invalid[1] = true;
    }
    if (elCdvty. ecWithComp[2] < 0.5) {
        recheckMessage = recheckMessage + (recheckMessage == NULL ? "" : ",") + chambers[2];
        invalidInput++;
        invalid[2] = true;
    }
    if (elCdvty.ecWithComp[3] < 0.5) {
        recheckMessage = recheckMessage + (recheckMessage == NULL ? "" : ",") + chambers[3];
        invalidInput++;
        invalid[3] = true;
    }

    tft.setBackgroundColor(COLOR_BLACK);
    tft.setFont(Terminal11x16);
    if (recheckMessage != "")
        tft.drawText(55 - (recheckMessage.length() * 5), 200, "Check " + recheckMessage, COLOR_RED);

    Serial.println("EMPTY CHAMBERS = " + String(invalidInput));
    return invalidInput;
}

void renderSensorCounter() {
    tft.setFont(Terminal11x16);
    tft.fillRectangle(0, 200, 180, 250, COLOR_BLACK);
    tft.drawText(25, 200, "Test No:  " + String(testsCounter));
}

void displayEC(ElectricalConductivity elCdvty, int invalidInput) {
    float minimum, lowerMinimum, upperMinimum, min1, min2;
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_BLACK);
    tft.drawText(150, 28, "mS");
    tft.drawText(5, 28, "EC");

    for (int i = 0; i < 4; i++) {
        elCdvty.ecWithComp[i] = int(elCdvty.ecWithComp[i] * 10.0f + (signbit(elCdvty.ecWithComp[i]) ? -0.5f : +0.5f)) / 10.0f;
        Serial.println(String(i) + " " + String(elCdvty.ecWithComp[i]));
    }

    tft.setFont(Terminal11x16);

    tft.drawText(10, 88, "FL");
    if (elCdvty.ecWithComp[0] >= 0.5)
        tft.drawText(40, 88, String(elCdvty.ecWithComp[0], 1) + "");
    else
        tft. drawText(40, 88, "0.00");
    Serial.println("Final EC 0 - " + String(elCdvty.ecWithComp[0]));

    tft.drawText(95, 88, "FR");
    if (elCdvty.ecWithComp[1] >= 0.5)
        tft.drawText(125, 88, String(elCdvty.ecWithComp[1], 1) + "");
    else
        tft.drawText(125, 88, "0.00");
    Serial.println("Final EC 1 - " + String(elCdvty.ecWithComp[1]));

    tft.drawText(10, 115, "BL");
    if (elCdvty.ecWithComp[2] >= 0.5)
        tft.drawText(40, 115, String(elCdvty.ecWithComp[2], 1) + "");
    else
        tft.drawText(40, 115, "0.00");
    Serial.println("Final EC 2 - " + String(elCdvty.ecWithComp[2]));

    tft.drawText(95, 115, "BR");
    if (elCdvty.ecWithComp[3] >= 0.5)
        tft.drawText(125, 115, String(elCdvty.ecWithComp[3], 1) + "");
    else
        tft.drawText(125, 115, "0.00");
    Serial.println("Final EC 3 - " + String(elCdvty.ecWithComp[3]));
    Serial.println("autocalFlah:  " + String(autoCalFlag));
    
    if (autoCalFlag) {
        if (calibrationA || ((elCdvty.ecWithComp[0] > 2.0 && elCdvty.ecWithComp[0] < 6.0) && 
                              (elCdvty.ecWithComp[1] > 2.0 && elCdvty.ecWithComp[1] < 6.0) && 
                              (elCdvty.ecWithComp[2] > 2.0 && elCdvty. ecWithComp[2] < 6.0) && 
                              (elCdvty.ecWithComp[3] > 2.0 && elCdvty.ecWithComp[3] < 6.0))) {
            tft.drawBitmap(30, 30, solA, 55, 55, COLOR_GREEN);
            tft.drawBitmap(90, 30, solA, 55, 55, COLOR_GREEN);
            tft.drawBitmap(30, 130, solA, 55, 55, COLOR_GREEN);
            tft.drawBitmap(90, 130, solA, 55, 55, COLOR_GREEN);
            delay(5000);
            return;
        }
        if (calibrationB || ((elCdvty.ecWithComp[0] > 6.0 && elCdvty.ecWithComp[0] < 10.0) && 
                              (elCdvty.ecWithComp[1] > 6.0 && elCdvty.ecWithComp[1] < 10.0) && 
                              (elCdvty.ecWithComp[2] > 6.0 && elCdvty. ecWithComp[2] < 10.0) && 
                              (elCdvty.ecWithComp[3] > 6.0 && elCdvty.ecWithComp[3] < 10.0))) {
            tft.drawBitmap(30, 30, solB, 55, 55, COLOR_GREEN);
            tft.drawBitmap(90, 30, solB, 55, 55, COLOR_GREEN);
            tft.drawBitmap(30, 130, solB, 55, 55, COLOR_GREEN);
            tft.drawBitmap(90, 130, solB, 55, 55, COLOR_GREEN);
            delay(5000);
            return;
        }
    }

    if (invalidInput <= 2) {
        if (invalid[0] == true)
            float min1 = elCdvty. ecWithComp[1];
        else if (invalid[1] == true)
            float min1 = elCdvty. ecWithComp[0];
        else
            min1 = min(elCdvty.ecWithComp[1], elCdvty.ecWithComp[0]);

        if (invalid[2] == true)
            float min2 = elCdvty.ecWithComp[3];
        else if (invalid[3] == true)
            float min2 = elCdvty.ecWithComp[2];
        else
            min2 = min(elCdvty.ecWithComp[2], elCdvty.ecWithComp[3]);
    }

    minimum = min(min1, min2);
    Serial.println("minimum: " + String(minimum));
    minimum = int(minimum * 10.0f + (signbit(minimum) ? -0.5f : +0.5f)) / 10.0f;

    const double EPSILON = 10e-8;
    bool SCflag = false;
    bool Cflag = false;

    if (fabs(minimum - 5.60) < EPSILON)
        SCflag = true;

    if (fabs(minimum - 7.50) < EPSILON)
        Cflag = true;

    if (minimum < 5.6 && SCflag == false) {
        Serial.println("NORMAL CUTTOFF");
        Serial.println("minimum" + String(minimum));
        lowerMinimum = minimum * (float)(115 / 100.0);
        Serial.println("lower minimum" + String(lowerMinimum));
        upperMinimum = minimum * (float)(140 / 100.0);
        Serial.println("upper minimum" + String(upperMinimum));
    } else if ((SCflag == true || minimum >= 5.6) && minimum < 7.5) {
        Serial.println("SUBCLINICAL CUTTOFF");
        Serial.println("minimum: " + String(minimum));
        lowerMinimum = minimum;
        Serial.println("lower minimum" + String(lowerMinimum));
        int percentage = newSCcuttoffs(minimum);
        percentage += 100;
        Serial.println("percentage: " + String(percentage));
        upperMinimum = lowerMinimum * (float)(percentage / 100.0);
        Serial.println("upper minimum" + String(upperMinimum));
    } else if (Cflag == true) {
        Serial.println("CLINICAL CUTTOFF");
        Serial.println("minimum: " + String(minimum));
        lowerMinimum = minimum;
        upperMinimum = minimum;
        Serial.println("upper minimum" + String(upperMinimum));
    } else {
    }

    tft.setFont(Terminal12x16);
    int i = 0;
    int j = 0;
    int k = 0;

    if ((elCdvty.ecWithComp[0] >= upperMinimum) && invalid[0] != true) {
        tft.drawBitmap(30, 30, clinical, 55, 55, COLOR_RED);
        positives[0][j] = "FL";
        resultFL += "CLINICAL";
        j++;
    } else if (elCdvty.ecWithComp[0] >= lowerMinimum && invalid[0] != true) {
        tft.drawBitmap(30, 30, subClinical, 55, 55, COLOR_YELLOW);
        positives[1][k] = "FL";
        resultFL += "SUB-CLINICAL";
        k++;
    } else if (invalid[0] != true) {
        tft.drawBitmap(30, 30, greenTick, 55, 55, COLOR_GREEN);
        resultFL += "NORMAL";
    } else {
        resultFL += "EMPTY";
        tft.drawBitmap(30, 30, noLiquid, 55, 55, COLOR_BLUE);
    }

    if ((elCdvty.ecWithComp[1] >= upperMinimum) && invalid[1] != true) {
        tft.drawBitmap(90, 30, clinical, 55, 55, COLOR_RED);
        positives[0][j] = "FR";
        resultFR += "CLINICAL";
        j++;
    } else if (elCdvty.ecWithComp[1] >= lowerMinimum && invalid[1] != true) {
        tft.drawBitmap(90, 30, subClinical, 55, 55, COLOR_YELLOW);
        positives[1][k] = "FR";
        resultFR += "SUB-CLINICAL";
        k++;
    } else if (invalid[1] != true) {
        tft.drawBitmap(90, 30, greenTick, 55, 55, COLOR_GREEN);
        resultFR += "NORMAL";
    } else {
        resultFR += "EMPTY";
        tft.drawBitmap(90, 30, noLiquid, 55, 55, COLOR_BLUE);
    }

    if ((elCdvty.ecWithComp[2] >= upperMinimum) && invalid[2] != true) {
        tft.drawBitmap(30, 130, clinical, 55, 55, COLOR_RED);
        positives[0][j] = "BL";
        resultBL += "CLINICAL";
        j++;
    } else if (elCdvty.ecWithComp[2] >= lowerMinimum && invalid[2] != true) {
        tft.drawBitmap(30, 130, subClinical, 55, 55, COLOR_YELLOW);
        positives[1][k] = "BL";
        resultBL += "SUB-CLINICAL";
        k++;
    } else if (invalid[2] != true) {
        tft.drawBitmap(30, 130, greenTick, 55, 55, COLOR_GREEN);
        resultBL += "NORMAL";
    } else {
        resultBL += "EMPTY";
        tft.drawBitmap(30, 130, noLiquid, 55, 55, COLOR_BLUE);
    }

    if ((elCdvty.ecWithComp[3] >= upperMinimum) && invalid[3] != true) {
        tft.drawBitmap(90, 130, clinical, 55, 55, COLOR_RED);
        positives[0][j] = "BR";
        resultBR += "CLINICAL";
        j++;
    } else if (elCdvty.ecWithComp[3] >= lowerMinimum && invalid[3] != true) {
        tft.drawBitmap(90, 130, subClinical, 55, 55, COLOR_YELLOW);
        positives[1][k] = "BR";
        resultBR += "SUB-CLINICAL";
        k++;
    } else if (invalid[3] != true) {
        tft.drawBitmap(90, 130, greenTick, 55, 55, COLOR_GREEN);
        resultBR += "NORMAL";
    } else {
        resultBR += "EMPTY";
        tft.drawBitmap(90, 130, noLiquid, 55, 55, COLOR_BLUE);
    }
    delay(3000);
}

float* displayTemp(float* temperatures) {
    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_BLACK);
    tft.drawText(50, 50, "FL");
    tft.drawText(17, 72, String(temperatures[0], 1) + "", COLOR_YELLOW);
    Serial.println("Final Temperature 0 - " + String(temperatures[0]));

    tft.drawText(105, 50, "FR");
    tft.drawText(95, 72, String(temperatures[1], 1) + "", COLOR_YELLOW);
    Serial.println("Final Temperature 1 - " + String(temperatures[1]));

    tft.drawText(50, 155, "BL");
    tft.drawText(17, 130, String(temperatures[2], 1) + "", COLOR_YELLOW);
    Serial.println("Final Temperature 2 - " + String(temperatures[2]));

    tft.drawText(105, 155, "BR");
    tft.drawText(95, 130, String(temperatures[3], 1) + "", COLOR_YELLOW);
    Serial.println("Final Temperature 3 - " + String(temperatures[3]));

    tft.drawText(7, 200, "Temperature - C", COLOR_YELLOW);
    tft.setFont(Terminal6x8);
    tft.drawText(155, 195, "o", COLOR_YELLOW);
    return temperatures;
}

void renderAutoCalibrationPage() {
    const float refRange = 0.0;

    tft.setFont(Terminal12x16);
    tft.setBackgroundColor(COLOR_BLACK);
    tft.drawText(65, 80, "AUTO", COLOR_YELLOW);
    tft.drawText(25, 105, "CALIBRATION", COLOR_YELLOW);
    tft.drawText(65, 130, "MODE", COLOR_YELLOW);
    delay(5000);
    tft.clear();
    skipForACSol();
    elCdvty = renderQuadmastestRoutine(elCdvty, temperatureLow);
    if (invalidInput > 0) {
        delay(5000);
        return;
    }
    tft.clear();
    tft.drawText(10, 105, "Calibrating...", COLOR_YELLOW);
    delay(4000);
    tft.clear();
    elCdvty = AutoCalibrationRoutine(elCdvty, temperatureLow);
    renderBatteryLevelAndQuadPie();
    displayEC(elCdvty, invalidInput);

    if (calibrationA) {
        tft.clear();
        tft.drawText(10, 105, " 'A' Calibration", COLOR_YELLOW);
        tft.drawText(80, 135, "DONE", COLOR_YELLOW);
        delay(5000);
    } else if (calibrationB) {
        tft.clear();
        tft.drawText(10, 105, " 'B' Calibration ", COLOR_YELLOW);
        tft.drawText(80, 135, "DONE", COLOR_YELLOW);
        delay(5000);
    } else {
        tft.clear();
        tft.drawText(10, 105, " ERROR!!  ", COLOR_YELLOW);
        delay(5000);
    }

    tft.clear();
}

void checkCharging() {
    int countdown = 5;
    while (digitalRead(CHARGER_VOLT) == HIGH && countdown > 0) {
        Serial.println("CHARGING DETECTED, pin reading: ");
        tft.setFont(Terminal12x16);
        tft.clear();
        tft.drawText(25, 95, "CHARGING.. .", COLOR_TOMATO);
        tft.drawText(10, 120, "SWITCHING OFF", COLOR_TOMATO);
        tft.drawText(30, 145, "in " + String(countdown) + " Sec", COLOR_GOLD);
        delay(1000);
        countdown--;
    }
    tft.clear();
    if (countdown == 0) {
        Serial.println(F("POWER OFF"));
        tft.setBacklight(false);
        tft.setDisplay(false);
        tft.clear();
        digitalWrite(POWER_LATCH_PIN, LOW);
    } else {
        return;
    }
}