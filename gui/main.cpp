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
#include <QtQuickControls2/QQuickStyle>
#include <QQmlApplicationEngine>
//#include <QtQuickControls2/QtQuickControls2>

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
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    app.setStyleSheet("QSlider::groove:horizontal {"
                          "border: 1px solid #999999;"
                          "height: 50px;"
                          "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #bbb, stop: 1 #bbf);"
                          "margin: 2px 0;"
                      "}"

                      "QSlider::handle:horizontal {"
                          "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);"
                          "border: 1px solid #5c5c5c;"
                          "width: 20px;"
                          "margin-top: -20px;"
                          "margin-bottom: -20px;"
                          "border-radius: 3px;"
                      "}"
                      );

    QQuickStyle::setStyle("Universal");
    QQmlApplicationEngine engine;
    engine.load(QUrl("qrc:/main.qml"));

    //Initialize global variables with default values
    g_isComfortable = true;
    g_setpoint_temperature = 72;
    g_heatCoolOutput = 0;
    g_indoorTemp = 72;
    g_outdoorTemp = 72;
    g_relHumidity = 50;
    g_globeTemp = 70;
    g_occupancy = true;
    g_pmv = 0;
    g_desiredTemp = 72;
    g_activityLevel = "resting";

    QApplication a(argc, argv);
    MainWindow w;
    //Run GUI application
    w.setupWindow();
    return a.exec();
}

