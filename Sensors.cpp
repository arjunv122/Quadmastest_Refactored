#include "Sensors.h"

float calculateTempWithNTC(int voltage) {
    float logR2, R2, T, Tc, Tf;
    float R1 = 100000, c1 = 2.114990448e-03, c2 = 0.3832381228e-04, c3 = 5.228061052e-07;
    R2 = R1 * (1023.0 / (float)voltage - 1.0);
    logR2 = log(R2);
    T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
    Tc = ((T - 273.15) - 2);
    return Tc;
}

float* measureTemp(float temperatures[]) {
    Serial.println("T 0 pin reading " + String(mcp. analogRead(TEMP_PIN1)));
    Serial.println("T 1 pin reading " + String(mcp.analogRead(TEMP_PIN2)));
    Serial.println("T 2 pin reading " + String(mcp. analogRead(TEMP_PIN3)));
    Serial.println("T 3 pin reading " + String(mcp.analogRead(TEMP_PIN4)));
    
    temperatures[0] = calculateTempWithNTC(mcp.analogRead(TEMP_PIN1));
    Serial.println("Temperature 0 - " + String(temperatures[0]));
    temperatures[1] = calculateTempWithNTC(mcp.analogRead(TEMP_PIN2));
    Serial.println("Temperature 1 - " + String(temperatures[1]));
    temperatures[2] = calculateTempWithNTC(mcp.analogRead(TEMP_PIN3));
    Serial.println("Temperature 2 - " + String(temperatures[2]));
    temperatures[3] = calculateTempWithNTC(mcp.analogRead(TEMP_PIN4));
    Serial.println("Temperature 3 - " + String(temperatures[3]));

    return temperatures;
}

ElectricalConductivity measureECWithoutComp(float* temperatures, ElectricalConductivity elCdvty) {
    int ECAnalogVal1, ECAnalogVal2, ECAnalogVal3, ECAnalogVal4;
    ADS. setGain(0);
    int16_t adc0;
    int16_t adc1;
    int16_t adc2;
    int16_t adc3;
    adc0 = ADS.readADC(0);
    adc1 = ADS.readADC(1);
    adc2 = ADS.readADC(2);
    adc3 = ADS.readADC(3);

    float f = ADS.toVoltage(10);

    Serial.println("EC 0 pin reading " + String(ADS.readADC(0)));
    Serial.println("EC 1 pin reading " + String(ADS.readADC(1)));
    Serial.println("EC 2 pin reading " + String(ADS.readADC(2)));
    Serial.println("EC 3 pin reading " + String(ADS.readADC(3)));

    elCdvty.ecWithoutComp[0] += (adc0 * f);
    elCdvty.ecWithoutComp[1] += (adc1 * f);
    elCdvty.ecWithoutComp[2] += (adc2 * f);
    elCdvty.ecWithoutComp[3] += (adc3 * f);

    Serial.println("EC 0 w/o Compensation- " + String(elCdvty.ecWithoutComp[0]));
    Serial.println("EC 1 w/o Compensation- " + String(elCdvty.ecWithoutComp[1]));
    Serial.println("EC 2 w/o Compensation- " + String(elCdvty. ecWithoutComp[2]));
    Serial.println("EC 3 w/o Compensation- " + String(elCdvty.ecWithoutComp[3]));

    return elCdvty;
}

int newSCcuttoffs(float minimum) {
    for (int i = 0; i < 15; i++) {
        if (minimum == SCmap. minimumValue[i]) {
            Serial.println("cuttoff percentage for clinical: " + String(SCmap.percentage[i]));
            Serial.println("i value" + String(i));
            return SCmap.percentage[i];
        }
    }
    return 0;
}