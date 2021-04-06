#include "homewindow.h"
#include <QApplication>
#include "measurements.h"
#include <string>
#include <QAbstractItemView>

HomeWindow::HomeWindow(QWidget *parent) : QWidget(parent)
{
    layout = new QGridLayout(this);
    tempDial = new QDial(this);
    desiredTemp = new QLCDNumber(this);
    exitButton = new QPushButton("Exit", this);
    exitButton->setFixedSize(BUTTON_SIZE);
    exitButton->setIcon(QIcon("/home/pi/WA/comfortControl/gui/icons/close-line.svg"));
    exitButton->setIconSize(ICON_SIZE);
    sensorsButton = new QPushButton("", this);
    sensorsButton->setFixedSize(BUTTON_SIZE);
    sensorsButton->setIcon(QIcon("/home/pi/WA/comfortControl/gui/icons/router-fill.svg"));
    sensorsButton->setIconSize(ICON_SIZE);
    settingsButton = new QPushButton(this);
    settingsButton->setFixedSize(BUTTON_SIZE);
    settingsButton->setIcon(QIcon("/home/pi/WA/comfortControl/gui/icons/settings-3-fill.svg"));
    settingsButton->setIconSize(ICON_SIZE);
    activityBox = new QComboBox(this);
    activityBox->setFixedSize(BUTTON_SIZE);
    activityBox->view()->setMinimumWidth(STANDARD_LENGTH);
    sensorsBar = new QHBoxLayout(this);
    currentTime = new QLCDNumber(this);
    currentTime->setDigitCount(8);
    QLabel *setToLbl = new QLabel("Set To",this);
    //Sensors Bar
    QIcon home("/home/pi/WA/comfortControl/gui/icons/home-2-fill.svg");
    homeImage = new QLabel();
    homeImage->setPixmap(home.pixmap(ICON_SIZE));
    QIcon outdoor("/home/pi/WA/comfortControl/gui/icons/sun-cloudy-line.svg");
    outdoorImage = new QLabel();
    outdoorImage->setPixmap(outdoor.pixmap(ICON_SIZE));
    QIcon humidity("/home/pi/WA/comfortControl/gui/icons/drop-fill.svg");
    humidityImage = new QLabel();
    humidityImage->setPixmap(humidity.pixmap(ICON_SIZE));
    indoorTemp = new QLCDNumber(this);
    outdoorTemp = new QLCDNumber(this);
    relHumidity = new QLCDNumber(this);
    sensorsBar->addStretch(2);
    sensorsBar->addWidget(homeImage);
    sensorsBar->addWidget(indoorTemp);
    sensorsBar->addStretch(2);
    sensorsBar->addWidget(outdoorImage);
    sensorsBar->addWidget(outdoorTemp);
    sensorsBar->addStretch(2);
    sensorsBar->addWidget(humidityImage);
    sensorsBar->addWidget(relHumidity);
    sensorsBar->addStretch(2);

    //Full Layout
    layout->addWidget(setToLbl, 0,2);
    layout->addWidget(desiredTemp,0,3,1,2);
    layout->addWidget(activityBox, 2,0);
    layout->addWidget(tempDial,1,1,3,5);
    layout->addWidget(currentTime,0,0,1,2);
    layout->addWidget(sensorsButton,4,0);
    layout->addWidget(settingsButton,4,6);
    layout->addWidget(exitButton, 0, 6);
    layout->addLayout(sensorsBar,4,1,1,5);
    this->setLayout(layout);
    tempDial->setMinimum(50);
    tempDial->setMaximum(90);
    tempDial->setValue(g_desiredTemp);
    desiredTemp->display(g_desiredTemp);
    QStringList activityOptions = {"resting", "moderately active", "active"};
    activityBox->addItems(activityOptions);
    QObject::connect(activityBox, &QComboBox::currentTextChanged, this, &HomeWindow::activityChanged);
    QObject::connect(tempDial, &QDial::valueChanged, this, &HomeWindow::desiredTempChanged);
    updateClock = new QTimer(this);
    QObject::connect(updateClock, &QTimer::timeout, [=] { currentTime->display(QTime::currentTime().toString("hh:mm:ss")); });
    updateClock->start(1000);
}

void HomeWindow::desiredTempChanged(int dt){
    desiredTemp->display(dt);
    g_desiredTemp = dt;
}

void HomeWindow::activityChanged(QString value){
    g_activityLevel = value.toStdString();
}

HomeWindow::~HomeWindow()
{
    delete layout;
    delete tempDial;
    delete desiredTemp;
    delete exitButton;
}
