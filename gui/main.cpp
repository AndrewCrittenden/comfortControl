#include "mainwindow.h"
#include <QApplication>
#include <QGuiApplication>
#include <QtConcurrent/QtConcurrent>
#include <QThread>
#include <unistd.h>
#include <stdio.h>
#include <QDebug>
#include "ControlAlgorithm.h"
#include "measurements.h"
#include <thread>
#include <QTime>

bool g_isComfortable;
double g_setpoint_temperature;
double g_heatCoolOutput;
double g_indoorTemp;
double g_outdoorTemp;
double g_relHumidity;
double g_globeTemp;
bool g_occupancy;
double g_pmv;
int g_desiredTemp;
std::string g_activityLevel;

int tick_get() {  // TODO Fix this to work with PID
    QTime time;
    return time.msecsSinceStartOfDay();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //Initialize global variables with default values
    g_isComfortable = true;
    g_setpoint_temperature = 72;
    g_heatCoolOutput = 0;
    g_indoorTemp = 72;
    g_outdoorTemp = 72;
    g_relHumidity = 0.5;
    g_globeTemp = 70;
    g_occupancy = true;
    g_pmv = 0;
    g_desiredTemp = 72;
    g_activityLevel = "resting";

    //Run GUI application
    w.setupWindow();
    return a.exec();
}

