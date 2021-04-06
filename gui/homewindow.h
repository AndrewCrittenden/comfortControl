#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QWidget>
#include <QLayout>
#include <QDial>
#include <QLCDNumber>
#include <QPushButton>
#include <QDebug>
#include <QSignalMapper>
#include <QComboBox>
#include <QTime>
#include <QTimer>
#include <QLabel>

class HomeWindow : public QWidget
{
public:
    explicit HomeWindow(QWidget *parent = nullptr);
    ~HomeWindow();
    QPushButton *sensorsButton;
    QPushButton *exitButton;
    QPushButton *settingsButton;
    QDial *tempDial;
    QComboBox *activityBox;
    QLCDNumber *indoorTemp;
    QLCDNumber *outdoorTemp;
    QLCDNumber *relHumidity;

private:
    QGridLayout *layout;
    QHBoxLayout *sensorsBar;
    QLCDNumber *desiredTemp;
    QLCDNumber *currentTime;
    QTimer *updateClock;
    QLabel *homeImage;
    QLabel *outdoorImage;
    QLabel *humidityImage;

public Q_SLOTS:
    void activityChanged(QString value);
    void desiredTempChanged(int dt);
};

#endif // HOMEWINDOW_H
