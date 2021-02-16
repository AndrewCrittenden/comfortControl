#include "sensorswindow.h"
#include <QApplication>

SensorsWindow::SensorsWindow(QWidget *parent) : QWidget(parent)
{
    layout = new QGridLayout(this);
    backButton = new QPushButton("Back", this);
    indoorTemp = new QLCDNumber(this);
    outdoorTemp = new QLCDNumber(this);
    absHumidity = new QLCDNumber(this);
    relHumidity = new QLCDNumber(this);
    globeTemp = new QLCDNumber(this);
    occupancy = new QLabel("Occupied", this);
    indoorTempLbl = new QLabel("Indoor Temperature", this);
    outdoorTempLbl = new QLabel("Outdoor Temperature", this);
    absHumidityLbl = new QLabel("Absolute Humidity", this);
    relHumidityLbl = new QLabel("Relative Humidity", this);
    globeTempLbl = new QLabel("Globe Temperature", this);
    occupancyLbl = new QLabel("Occupancy", this);
    layout->addWidget(indoorTempLbl,1,1);
    layout->addWidget(indoorTemp,1,2);
    layout->addWidget(outdoorTempLbl,2,1);
    layout->addWidget(outdoorTemp,2,2);
    layout->addWidget(absHumidityLbl,3,1);
    layout->addWidget(absHumidity,3,2);
    layout->addWidget(relHumidityLbl,4,1);
    layout->addWidget(relHumidity,4,2);
    layout->addWidget(globeTempLbl,5,1);
    layout->addWidget(globeTemp,5,2);
    layout->addWidget(occupancyLbl,6,1);
    layout->addWidget(occupancy,6,2);
    layout->addWidget(backButton,0,3);
    this->setLayout(layout);
}

SensorsWindow::~SensorsWindow()
{
    delete layout;
    delete backButton;
}

