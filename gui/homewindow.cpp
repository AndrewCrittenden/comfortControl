#include "homewindow.h"
#include <QApplication>
#include "measurements.h"
#include <string>
#include <QAbstractItemView>

HomeWindow::HomeWindow(QWidget *parent) : QWidget(parent)
{
    layout = new QGridLayout(this);
    tempDial = new QDial(this);
    desiredTemp = new QLCDNumber(2,this);
    desiredTemp->setFrameStyle(QFrame::NoFrame);
    exitButton = new QPushButton("", this);
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
    activityBox->setFixedSize(QSize(2*STANDARD_LENGTH,STANDARD_LENGTH));
    activityBox->view()->setMinimumWidth(STANDARD_LENGTH);
    applianceState = new QLabel("Fridge Off");
    indoorTempUnits = new QLabel();
    indoorTempUnits->setPixmap(fahrenheitIcon->pixmap(ICON_SIZE_SMALL));
    outdoorTempUnits = new QLabel();
    outdoorTempUnits->setPixmap(fahrenheitIcon->pixmap(ICON_SIZE_SMALL));
    relHumidityUnits = new QLabel();
    relHumidityUnits->setPixmap(percentIcon->pixmap(ICON_SIZE_SMALL));
    desiredTempUnits = new QLabel();
    desiredTempUnits->setPixmap(fahrenheitIcon->pixmap(ICON_SIZE_SMALL));
    sensorsBar = new QHBoxLayout(this);
    currentTime = new QLCDNumber(this);
    currentTime->setFrameStyle(QFrame::NoFrame);
    currentTime->setDigitCount(8);
    QLabel *setToLbl = new QLabel("Set To",this);
    QFont font = setToLbl->font();
    font.setPointSize(20);
    setToLbl->setFont(font);
    activityBox->setFont(font);
    applianceState->setFont(font);
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
    indoorTemp = new QLCDNumber(2,this);
    indoorTemp->setFrameStyle(QFrame::NoFrame);
    outdoorTemp = new QLCDNumber(2,this);
    outdoorTemp->setFrameStyle(QFrame::NoFrame);
    relHumidity = new QLCDNumber(2,this);
    relHumidity->setFrameStyle(QFrame::NoFrame);
    sensorsBar->addStretch(2);
    sensorsBar->addWidget(homeImage);
    sensorsBar->addWidget(indoorTemp);
    sensorsBar->addWidget(indoorTempUnits);
    sensorsBar->addStretch(2);
    sensorsBar->addWidget(outdoorImage);
    sensorsBar->addWidget(outdoorTemp);
    sensorsBar->addWidget(outdoorTempUnits);
    sensorsBar->addStretch(2);
    sensorsBar->addWidget(humidityImage);
    sensorsBar->addWidget(relHumidity);
    sensorsBar->addWidget(relHumidityUnits);
    sensorsBar->addStretch(2);

    //Full Layout
    currentTime->setGeometry(0,0,2*STANDARD_LENGTH,STANDARD_LENGTH);
    layout->addWidget(setToLbl, 0,2, Qt::AlignRight);
    layout->addWidget(desiredTemp,0,3);
    layout->addWidget(desiredTempUnits,0,4);
    layout->addWidget(activityBox, 2,0,1,2);
    layout->addWidget(applianceState,2,5,1,2);
    layout->addWidget(tempDial,1,1,3,5);
    //layout->addWidget(currentTime,0,0,1,2);
    layout->addWidget(sensorsButton,4,0);
    layout->addWidget(settingsButton,4,6);
    layout->addWidget(exitButton, 0, 6);
    layout->addLayout(sensorsBar,4,1,1,5);
    this->setLayout(layout);
    tempDial->setMinimum(50);
    tempDial->setMaximum(90);
    tempDial->setValue(g_desiredTemp);
    desiredTemp->display(g_desiredTemp);
    QStringList activityOptions = {"Resting", "Moderately\nActive", "Active"};
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
    if(value == "Resting"){
        g_activityLevel = "resting";
    }
    else if(value == "Moderately\nActive"){
        g_activityLevel = "moderately active";
    }
    else if(value == "Active"){
        g_activityLevel = "active";
    }
}

HomeWindow::~HomeWindow()
{
    delete layout;
    delete tempDial;
    delete desiredTemp;
    delete exitButton;
}
