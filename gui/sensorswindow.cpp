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
    //indoorTemp->setFrameStyle(QFrame::NoFrame);
    outdoorTemp = new QLCDNumber(this);
    //outdoorTemp->setFrameStyle(QFrame::NoFrame);
    relHumidity = new QLCDNumber(this);
    //relHumidity->setFrameStyle(QFrame::NoFrame);
    globeTemp = new QLCDNumber(this);
    //globeTemp->setFrameStyle(QFrame::NoFrame);
    occupancy = new QLabel("Occupied", this);
    occupancy->setFrameStyle(QFrame::Box | QFrame::Raised);
    setpointTemp = new QLCDNumber(this);
    //setpointTemp->setFrameStyle(QFrame::NoFrame);
    pmv = new QLCDNumber(this);
    //pmv->setFrameStyle(QFrame::NoFrame);
    heatCoolOutput = new QLCDNumber(this);
    //heatCoolOutput->setFrameStyle(QFrame::NoFrame);
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
    authenticateButton = new QPushButton("Connect\nNodes", this);
    authenticateButton->setFixedSize(BUTTON_SIZE);
    clearNodeButton = new QPushButton("Clear\nNodes", this);
    clearNodeButton->setFixedSize(BUTTON_SIZE);
    refreshBar = new QProgressBar(this);
    refreshBar->setGeometry(0,470,800,10);
    //Add units
    indoorUnits = new QLabel();
    indoorUnits->setPixmap(fahrenheitIcon->pixmap(ICON_SIZE_SMALL));
    outdoorUnits = new QLabel();
    outdoorUnits->setPixmap(fahrenheitIcon->pixmap(ICON_SIZE_SMALL));
    relHumidityUnits = new QLabel();
    relHumidityUnits->setPixmap(percentIcon->pixmap(ICON_SIZE_SMALL));
    globeUnits = new QLabel();
    globeUnits->setPixmap(fahrenheitIcon->pixmap(ICON_SIZE_SMALL));
    setpointTempUnits = new QLabel();
    setpointTempUnits->setPixmap(fahrenheitIcon->pixmap(ICON_SIZE_SMALL));
    applianceUnits = new QLabel("Watts",this);

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
    occupancy->setFont(font);
    authenticateButton->setFont(font);
    clearNodeButton->setFont(font);
    applianceUnits->setFont(font);

    //Add widgets to layout
    layout->addWidget(indoorTempLbl,0,1);
    layout->addWidget(indoorTemp,0,2);
    layout->addWidget(indoorUnits,0,3);
    layout->addWidget(indoorTempSatusLbl,0,4);
    layout->addWidget(outdoorTempLbl,1,1);
    layout->addWidget(outdoorTemp,1,2);
    layout->addWidget(outdoorUnits,1,3);
    layout->addWidget(outdoorTempSatusLbl,1,4);
    layout->addWidget(relHumidityLbl,2,1);
    layout->addWidget(relHumidity,2,2);
    layout->addWidget(relHumidityUnits,2,3);
    layout->addWidget(relHumiditySatusLbl,2,4);
    layout->addWidget(globeTempLbl,3,1);
    layout->addWidget(globeTemp,3,2);
    layout->addWidget(globeUnits,3,3);
    layout->addWidget(globeTempSatusLbl,3,4);
    layout->addWidget(occupancyLbl,4,1);
    layout->addWidget(occupancy,4,2);
    layout->addWidget(occupancySatusLbl,4,4);
    layout->addWidget(setpointTempLbl,5,1);
    layout->addWidget(setpointTemp,5,2);
    layout->addWidget(setpointTempUnits,5,3);
    layout->addWidget(pmvLbl,6,1);
    layout->addWidget(pmv,6,2);
    layout->addWidget(heatCoolOutputLbl,7,1);
    layout->addWidget(heatCoolOutput,7,2);
    layout->addWidget(applianceUnits,7,3);
    layout->addWidget(authenticateButton,0,0,2,1);
    layout->addWidget(clearNodeButton,3,0,2,1);
    layout->addWidget(backButton,6,0,2,1);
    this->setLayout(layout);
}

SensorsWindow::~SensorsWindow()
{
    delete layout;
    delete backButton;
}

