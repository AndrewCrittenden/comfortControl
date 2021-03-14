#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStackedWidget>
#include <QMainWindow>
#include "sensorswindow.h"
#include "homewindow.h"

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

private:
    QStackedWidget *stack;
    SensorsWindow *sensors;

signals:
    void exitApp();

public slots:
    void setWindow(QWidget *w);
    void refreshMeasurements();
    void IPCRecieveComfort();
    void IPCSendComfort(int dt);
};

#endif // MAINWINDOW_H
