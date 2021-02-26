#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QWidget>
#include <QLayout>
#include <QDial>
#include <QLCDNumber>
#include <QPushButton>
#include <QDebug>
#include <QSignalMapper>

class HomeWindow : public QWidget
{
public:
    explicit HomeWindow(QWidget *parent = nullptr);
    ~HomeWindow();
    QPushButton *sensorsButton;
    QPushButton *exitButton;
    QDial *tempDial;

private:
    QGridLayout *layout;
    QLCDNumber *desiredTemp;

signals:
    int sensorsButtonPressed();
};

#endif // HOMEWINDOW_H
