#include "sensorswindow.h"
#include <QApplication>
#include "measurements.h"
#include <QFont>

SensorsWindow::SensorsWindow(QWidget *parent) : QWidget(parent)
{
    layout = new QGridLayout(this);
    backButton = new QPushButton("", this);
    backButton->setFixedSize(BUTTON_SIZE);
    backButton->setIcon(QIcon("/home/pi/WA/comfortControl/gui/icons/arrow-go-back-line.svg"));
    backButton->setIconSize(ICON_SIZE);
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
    //Increase size of font
    QFont font = indoorTempLbl->font();
    font.setPointSize(20);
    indoorTempLbl->setFont(font);
    outdoorTempLbl->setFont(font);
    relHumidityLbl->setFont(font);
    globeTempLbl->setFont(font);
    occupancyLbl->setFont(font);
    setpointTempLbl->setFont(font);
    pmvLbl->setFont(font);
    heatCoolOutputLbl->setFont(font);
    indoorTempSatusLbl->setFont(font);
    outdoorTempSatusLbl->setFont(font);
    relHumiditySatusLbl->setFont(font);
    globeTempSatusLbl->setFont(font);
    occupancySatusLbl->setFont(font);
    layout->addWidget(indoorTempLbl,0,1);
    layout->addWidget(indoorTemp,0,2);
    layout->addWidget(indoorTempSatusLbl,0,3);
    layout->addWidget(outdoorTempLbl,1,1);
    layout->addWidget(outdoorTemp,1,2);
    layout->addWidget(outdoorTempSatusLbl,1,3);
    layout->addWidget(relHumidityLbl,2,1);
    layout->addWidget(relHumidity,2,2);
    layout->addWidget(relHumiditySatusLbl,2,3);
    layout->addWidget(globeTempLbl,3,1);
    layout->addWidget(globeTemp,3,2);
    layout->addWidget(globeTempSatusLbl,3,3);
    layout->addWidget(occupancyLbl,4,1);
    layout->addWidget(occupancy,4,2);
    layout->addWidget(occupancySatusLbl,4,3);
    layout->addWidget(setpointTempLbl,5,1);
    layout->addWidget(setpointTemp,5,2);
    layout->addWidget(pmvLbl,6,1);
    layout->addWidget(pmv,6,2);
    layout->addWidget(heatCoolOutputLbl,7,1);
    layout->addWidget(heatCoolOutput,7,2);
    layout->addWidget(backButton,6,0,2,1);
    this->setLayout(layout);
}

SensorsWindow::~SensorsWindow()
{
    delete layout;
    delete backButton;
}

