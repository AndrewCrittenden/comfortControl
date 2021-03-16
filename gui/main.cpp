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
    //Start PID thread
    float controlled_air_volume = 0.015; // I don't have the actual length/width/height handy right now
    const float max_cool = -2500;
    const float max_heat = 2500;
    ControlAlgorithm controller(74*controlled_air_volume, 1.1111*controlled_air_volume, 740*controlled_air_volume, max_cool, max_heat, 15000); // REQUIRED
    controller.setInitial(18.3333,22.2222222222); // REQUIRED
    std::thread controlLoop(&ControlAlgorithm::beginAlgorithmLoop,&controller); // REQUIRED
    qDebug() << controller.getOutput();
    controller.exitFlag = true; // REQUIRED
    controlLoop.join(); // REQUIRED
    //QFuture<void> t1 = QtConcurrent::run(controller, &ControlAlgorithm::beginAlgorithmLoop);
    //Start comfortAnalysis process
    QString comfortAnalysisPath = "/home/pi/WA/comfortControl/comfortCode/comfortAnalysis3152021.py";
    QProcess *comfortAnalysis = new QProcess;
    comfortAnalysis->start(comfortAnalysisPath);
    //Run GUI application
    w.setupWindow();
    return a.exec();
}
