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

signals:
    void exitApp();

public slots:
    void setWindow(QWidget *w);
    void refreshMeasurements();
    void IPCRecieveComfort();
    void IPCSendComfort(int dt);
};

#endif // MAINWINDOW_H
