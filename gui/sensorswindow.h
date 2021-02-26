#ifndef SENSORSWINDOW_H
#define SENSORSWINDOW_H

#include <QWidget>
#include <QLayout>
#include <QLCDNumber>
#include <QPushButton>
#include <QDebug>
#include <QLabel>

class SensorsWindow : public QWidget
{
public:
    explicit SensorsWindow(QWidget *parent = nullptr);
    ~SensorsWindow();
    QPushButton *backButton;
    QLCDNumber *indoorTemp;
    QLCDNumber *outdoorTemp;
    QLCDNumber *absHumidity;
    QLCDNumber *relHumidity;
    QLCDNumber *globeTemp;
    QLabel *occupancy;
    QLCDNumber *pmv;

private:
    QGridLayout *layout;
    QLabel *indoorTempLbl;
    QLabel *outdoorTempLbl;
    QLabel *absHumidityLbl;
    QLabel *relHumidityLbl;
    QLabel *globeTempLbl;
    QLabel *occupancyLbl;
    QLabel *pmvLbl;
};

#endif // SENSORSWINDOW_H
