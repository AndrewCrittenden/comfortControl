#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include <iostream>

extern bool g_isComfortable;
extern float g_setpoint_temperature; // unit fahrenheit
extern float g_heatCoolOutput; // unit Watts
extern float g_indoorTemp; // unit fahrenheit
extern float g_outdoorTemp; // unit fahrenheit
extern float g_relHumidity; // unit %
extern float g_globeTemp; // unit fahrenheit
extern bool g_occupancy;
extern float g_pmv; // unit range -3(cold) to 3(hot), -0.5 to 0.5 is comfortable
extern int g_desiredTemp; // unit fahrenheit
extern std::string g_activityLevel; //"resting", "moderately active", or "active"

#endif // MEASUREMENTS_H
