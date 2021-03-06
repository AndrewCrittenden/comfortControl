#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include <iostream>
#include <QSize>
#include <QIcon>

//GUI formatting constants
static QIcon *fahrenheitIcon = new QIcon("/home/pi/WA/comfortControl/gui/icons/fahrenheit-line.svg");
static QIcon *percentIcon = new QIcon("/home/pi/WA/comfortControl/gui/icons/percent-line.svg");
const int STANDARD_LENGTH = 110;
const QSize BUTTON_SIZE = QSize(STANDARD_LENGTH, STANDARD_LENGTH);
const QSize ICON_SIZE = QSize(70, 70);
const QSize ICON_SIZE_SMALL = QSize(30, 30);

extern bool g_isComfortable;
extern double g_setpoint_temperature; // unit fahrenheit
extern double g_heatCoolOutput; // unit Watts
extern double g_indoorTemp; // unit fahrenheit
extern double g_outdoorTemp; // unit fahrenheit
extern double g_relHumidity; // unit %
extern double g_globeTemp; // unit fahrenheit
extern bool g_occupancy;
extern double g_pmv; // unit range -3(cold) to 3(hot), -0.5 to 0.5 is comfortable
extern int g_desiredTemp; // unit fahrenheit
extern std::string g_activityLevel; //"resting", "moderately active", or "active"

#endif // MEASUREMENTS_H
