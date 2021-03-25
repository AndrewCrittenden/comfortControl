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
    static constexpr float controlled_air_volume = 0.015; // I don't have the actual length/width/height handy right now
    static constexpr float max_cool = -2500;
    static constexpr float max_heat = 2500;

signals:
    void exitApp();

public slots:
    void setWindow(QWidget *w);
    void refreshMeasurements();
    void IPCRecieveComfort();
    void IPCSendComfort(int dt);
};

#endif // MAINWINDOW_H
