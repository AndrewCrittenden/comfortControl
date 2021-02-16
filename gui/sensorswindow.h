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

private:
    QGridLayout *layout;
    QLCDNumber *indoorTemp;
    QLCDNumber *outdoorTemp;
    QLCDNumber *absHumidity;
    QLCDNumber *relHumidity;
    QLCDNumber *globeTemp;
    QLabel *occupancy;
    QLabel *indoorTempLbl;
    QLabel *outdoorTempLbl;
    QLabel *absHumidityLbl;
    QLabel *relHumidityLbl;
    QLabel *globeTempLbl;
    QLabel *occupancyLbl;
};

#endif // SENSORSWINDOW_H
