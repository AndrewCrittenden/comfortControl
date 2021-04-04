#include "sensorswindow.h"
#include <QApplication>

SensorsWindow::SensorsWindow(QWidget *parent) : QWidget(parent)
{
    layout = new QGridLayout(this);
    backButton = new QPushButton("Back", this);
    indoorTemp = new QLCDNumber(this);
    outdoorTemp = new QLCDNumber(this);
    relHumidity = new QLCDNumber(this);
    globeTemp = new QLCDNumber(this);
    occupancy = new QLabel("Occupied", this);
    setpointTemp = new QLCDNumber(this);
    pmv = new QLCDNumber(this);
    heatCoolOutput = new QLCDNumber(this);
    indoorTempLbl = new QLabel("Indoor Temperature", this);
    outdoorTempLbl = new QLabel("Outdoor Temperature", this);
    relHumidityLbl = new QLabel("Relative Humidity", this);
    globeTempLbl = new QLabel("Globe Temperature", this);
    occupancyLbl = new QLabel("Occupancy", this);
    setpointTempLbl = new QLabel("Setpoint Temperature", this);
    pmvLbl = new QLabel("Predicted Mean Vote", this);
    heatCoolOutputLbl = new QLabel("Appliance Wattage", this);
    indoorTempSatusLbl = new QLabel("Disconnected",this);
    outdoorTempSatusLbl = new QLabel("Disconnected",this);
    relHumiditySatusLbl = new QLabel("Disconnected",this);
    globeTempSatusLbl = new QLabel("Disconnected",this);
    occupancySatusLbl = new QLabel("Disconnected",this);
    layout->addWidget(indoorTempLbl,1,1);
    layout->addWidget(indoorTemp,1,2);
    layout->addWidget(indoorTempSatusLbl,1,3);
    layout->addWidget(outdoorTempLbl,2,1);
    layout->addWidget(outdoorTemp,2,2);
    layout->addWidget(outdoorTempSatusLbl,2,3);
    layout->addWidget(relHumidityLbl,3,1);
    layout->addWidget(relHumidity,3,2);
    layout->addWidget(relHumiditySatusLbl,3,3);
    layout->addWidget(globeTempLbl,4,1);
    layout->addWidget(globeTemp,4,2);
    layout->addWidget(globeTempSatusLbl,4,3);
    layout->addWidget(occupancyLbl,5,1);
    layout->addWidget(occupancy,5,2);
    layout->addWidget(occupancySatusLbl,5,3);
    layout->addWidget(setpointTempLbl,6,1);
    layout->addWidget(setpointTemp,6,2);
    layout->addWidget(pmvLbl,7,1);
    layout->addWidget(pmv,7,2);
    layout->addWidget(heatCoolOutputLbl,8,1);
    layout->addWidget(heatCoolOutput,8,2);
    layout->addWidget(backButton,0,3);
    this->setLayout(layout);
}

SensorsWindow::~SensorsWindow()
{
    delete layout;
    delete backButton;
}

