#include "settingswindow.h"

settingswindow::settingswindow(QWidget *parent) : QWidget(parent)
{
    layout = new QGridLayout(this);
    authenticateButton = new QPushButton("Authenticate", this);
    backButton = new QPushButton("Back", this);
    layout->addWidget(authenticateButton,1,1);
    layout->addWidget(backButton,0,3);
    this->setLayout(layout);
}

settingswindow::~settingswindow()
{
    delete layout;
    delete authenticateButton;
    delete backButton;
}
