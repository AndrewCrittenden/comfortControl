#include "homewindow.h"
#include <QApplication>
#include "measurements.h"
#include <string>

HomeWindow::HomeWindow(QWidget *parent) : QWidget(parent)
{
    layout = new QGridLayout(this);
    tempDial = new QDial(this);
    desiredTemp = new QLCDNumber(this);
    exitButton = new QPushButton("Exit", this);
    sensorsButton = new QPushButton("Sensors Status", this);
    settingsButton = new QPushButton("Settings",this);
    activityBox = new QComboBox(this);
    currentTime = new QLCDNumber(this);
    currentTime->setDigitCount(8);
    layout->addWidget(desiredTemp,0,1);
    layout->addWidget(activityBox, 1,0);
    layout->addWidget(tempDial,1,1);
    layout->addWidget(currentTime,0,2);
    layout->addWidget(sensorsButton,0,0);
    layout->addWidget(settingsButton,1,2);
    this->setLayout(layout);
    QStringList activityOptions = {"resting", "moderately active", "active"};
    activityBox->addItems(activityOptions);
    QObject::connect(tempDial, SIGNAL(valueChanged(int)), desiredTemp, SLOT(display(int)));
    QObject::connect(activityBox, &QComboBox::currentTextChanged, this, &HomeWindow::activityChanged);
    updateClock = new QTimer(this);
    QObject::connect(updateClock, &QTimer::timeout, [=] { currentTime->display(QTime::currentTime().toString("hh:mm:ss")); });
    updateClock->start(1000);
}

void HomeWindow::activityChanged(QString value){
    g_activityLevel = value.toStdString();
    qDebug() << "activity set to ";
    std::cout << g_activityLevel;
}

HomeWindow::~HomeWindow()
{
    delete layout;
    delete tempDial;
    delete desiredTemp;
    delete exitButton;
}
