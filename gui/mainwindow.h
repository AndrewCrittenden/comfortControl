#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStackedWidget>
#include <QMainWindow>
#include "sensorswindow.h"
#include "homewindow.h"
#include "settingswindow.h"
#include "measurements.h"
#include "servercomfort.h"
#include "ControlAlgorithm.h"
#include <python3.7m/Python.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setupWindow();
    ~MainWindow();
    HomeWindow *home;
    double indoorTemp, outdoorTemp, absHumidity, relHumidity, globeTemp, pmv;
    bool occupancy;
    serverCOMFORT server;

private:
    QStackedWidget *stack;
    SensorsWindow *sensors;
    settingswindow *settings;
    ControlAlgorithm controller;
    PyObject *pName, *pModule, *pFunc;

Q_SIGNALS:
    void exitApp();

public Q_SLOTS:
    void updateGatherFreq(int f);
    void updatePMV(bool doUpdate);
    void setWindow(QWidget *w);
    void refreshMeasurements();
    void indoorStatus(bool value);
    void outdoorStatus(bool value);
    void relHumStatus(bool value);
    void globeStatus(bool value);
    void occupancyStatus(bool value);
    void outputStatus(bool value);

};

#endif // MAINWINDOW_H
