#include "settingswindow.h"
#include "mainwindow.h"

const int DEFAULT_GATHERFREQ = 10;
const int MIN_GATHERFREQ = 5;
const int MAX_GATHERFREQ = 300;

settingswindow::settingswindow(QWidget *parent) : QWidget(parent)
{
    layout = new QGridLayout(this);
    backButton = new QPushButton("", this);
    backButton->setFixedSize(BUTTON_SIZE);
    backButton->setIcon(QIcon("/home/pi/WA/comfortControl/gui/icons/arrow-go-back-line.svg"));
    backButton->setIconSize(ICON_SIZE);
    gatherFreqSlider = new QSlider(Qt::Horizontal, this);
    gatherFreqSlider->setRange(MIN_GATHERFREQ, MAX_GATHERFREQ);
    gatherFreqSlider->setValue(DEFAULT_GATHERFREQ);
    gatherFreqSlider->setSingleStep(1);
    gatherFreqSlider->setGeometry(0,0,40,60);
    gatherFreqLbl = new QLabel("Measurement Resolution",this);
    gatherFreqDsp = new QLCDNumber(this);
    QTime t(0, DEFAULT_GATHERFREQ, 0);
    gatherFreqDsp->display(t.toString("m:ss"));

    //Increase size of font
    QFont font = gatherFreqLbl->font();
    font.setPointSize(20);
    gatherFreqLbl->setFont(font);

    layout->addWidget(gatherFreqLbl,0,0);
    layout->addWidget(gatherFreqDsp,0,1);
    layout->addWidget(gatherFreqSlider,0,2);
    layout->addWidget(backButton,4,7);
    this->setLayout(layout);
}

settingswindow::~settingswindow()
{
    delete layout;
    delete backButton;
}
