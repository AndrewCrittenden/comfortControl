#ifndef SENSORSWINDOW_H
#define SENSORSWINDOW_H

#include <QWidget>
#include <QLayout>
#include <QLCDNumber>
#include <QPushButton>
#include <QDebug>
#include <QLabel>
#include <QProgressBar>

class SensorsWindow : public QWidget
{
public:
    explicit SensorsWindow(QWidget *parent = nullptr);
    ~SensorsWindow();
    QPushButton *backButton;
    QLCDNumber *indoorTemp;
    QLCDNumber *outdoorTemp;
    QLCDNumber *relHumidity;
    QLCDNumber *globeTemp;
    QLabel *occupancy;
    QLCDNumber *setpointTemp;
    QLCDNumber *pmv;
    QLCDNumber *heatCoolOutput;
    QLabel *indoorTempSatusLbl;
    QLabel *outdoorTempSatusLbl;
    QLabel *relHumiditySatusLbl;
    QLabel *globeTempSatusLbl;
    QLabel *occupancySatusLbl;
    QPushButton *authenticateButton;
    QPushButton *clearNodeButton;
    QProgressBar *refreshBar;
    QLabel *indoorUnits;
    QLabel *outdoorUnits;
    QLabel *relHumidityUnits;
    QLabel *globeUnits;
    QLabel *setpointTempUnits;
    QLabel *applianceUnits;

private:
    QGridLayout *layout;
    QLabel *indoorTempLbl;
    QLabel *outdoorTempLbl;
    QLabel *relHumidityLbl;
    QLabel *globeTempLbl;
    QLabel *occupancyLbl;
    QLabel *setpointTempLbl;
    QLabel *pmvLbl;
    QLabel *heatCoolOutputLbl;
};

#endif // SENSORSWINDOW_H
