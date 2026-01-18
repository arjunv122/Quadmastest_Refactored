#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "Config.h"

// Function declarations
void autoCalBegin();
struct ElectricalConductivity AutoCalibrationRoutine(struct ElectricalConductivity elCdvty, float temperature[4]);
struct ElectricalConductivity calibrateECValues(struct ElectricalConductivity elCdvty, float temperature[4]);
void skipForACSol();
struct ElectricalConductivity renderManCal(struct ElectricalConductivity elCdvty, float temperatures[4]);

#endif