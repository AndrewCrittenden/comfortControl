#include "settingswindow.h"
#include "mainwindow.h"

const int DEFAULT_GATHERFREQ = 10;
const int MIN_GATHERFREQ = 5;
const int MAX_GATHERFREQ = 300;

settingswindow::settingswindow(QWidget *parent) : QWidget(parent)
{
    layout = new QGridLayout(this);
    authenticateButton = new QPushButton("Authenticate", this);
    authenticateButton->setFixedSize(BUTTON_SIZE);
    clearNodeButton = new QPushButton("Clear Nodes", this);
    clearNodeButton->setFixedSize(BUTTON_SIZE);
    backButton = new QPushButton("", this);
    backButton->setFixedSize(BUTTON_SIZE);
    backButton->setIcon(QIcon("/home/pi/WA/comfortControl/gui/icons/arrow-go-back-line.svg"));
    backButton->setIconSize(ICON_SIZE);
    gatherFreqSlider = new QSlider(Qt::Horizontal, this);
    gatherFreqSlider->setRange(MIN_GATHERFREQ, MAX_GATHERFREQ);
    gatherFreqSlider->setValue(DEFAULT_GATHERFREQ);
    gatherFreqSlider->setSingleStep(1);
    gatherFreqLbl = new QLabel("Measurement Resolution",this);
    gatherFreqDsp = new QLCDNumber(this);
    QTime t(0, DEFAULT_GATHERFREQ, 0);
    gatherFreqDsp->display(t.toString("m:ss"));
    layout->addWidget(authenticateButton,1,2);
    layout->addWidget(clearNodeButton, 2, 2);
    layout->addWidget(gatherFreqLbl,3,1);
    layout->addWidget(gatherFreqDsp,3,2);
    layout->addWidget(gatherFreqSlider,3,3);
    layout->addWidget(backButton,0,3);
    this->setLayout(layout);
}

settingswindow::~settingswindow()
{
    delete layout;
    delete authenticateButton;
    delete backButton;
}
