#include "mainwindow.h"
#include <QApplication>
#include <QGuiApplication>
#include <QtConcurrent/QtConcurrent>
#include <QThread>
#include <unistd.h>
#include <stdio.h>
#include <QDebug>
#include "PIDmain.cpp" //TODO update this to be a header file instead
#include "measurements.h"

bool g_isComfortable;
float g_setpoint_temperature;
float g_heatCoolOutput;
float g_indoorTemp;
float g_outdoorTemp;
float g_relHumidity;
float g_globeTemp;
bool g_occupancy;
float g_pmv;
int g_desiredTemp;
std::string g_activityLevel;

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
    //Start PID thread
    QFuture<void> t1 = QtConcurrent::run(PIDmain);
    //Start comfortAnalysis process
    QString comfortAnalysisPath = "/home/pi/WA/comfortControl/comfortCode/calc_pmv_ppd_senior_deisgn.py";
    QString comfortAnalysisDemoPath = "/home/pi/WA/comfortControl/comfortCode/comfortAnalysisDemo.py";
    QProcess *comfortAnalysis = new QProcess;
    comfortAnalysis->start(comfortAnalysisDemoPath);
    //Run GUI application
    w.setupWindow();
    return a.exec();
}
