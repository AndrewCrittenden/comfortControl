#include "settingswindow.h"

settingswindow::settingswindow(QWidget *parent) : QWidget(parent)
{
    layout = new QGridLayout(this);
    authenticateButton = new QPushButton("Authenticate", this);
    clearNodeButton = new QPushButton("Clear Nodes", this);
    backButton = new QPushButton("Back", this);
    gatherFreqSlider = new QSlider(Qt::Horizontal, this);
    gatherFreqSlider->setRange(5, 300);
    gatherFreqSlider->setSingleStep(1);
    gatherFreqLbl = new QLabel("Measurement Resolution",this);
    layout->addWidget(authenticateButton,1,2);
    layout->addWidget(clearNodeButton, 2, 2);
    layout->addWidget(gatherFreqSlider,3,2);
    layout->addWidget(gatherFreqLbl,3,1);
    layout->addWidget(backButton,0,3);
    this->setLayout(layout);
}

settingswindow::~settingswindow()
{
    delete layout;
    delete authenticateButton;
    delete backButton;
}
