#include "mainwindow.h"
#include <QCoreApplication>

//MainWindow is the controller used to switch between windows and close the application

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    stack = new QStackedWidget;
    home = new HomeWindow;
    sensors = new SensorsWindow;
    stack->addWidget(home);
    stack->addWidget(sensors);
    stack->setCurrentWidget(home);
    //stack->setWindowState(Qt::WindowFullScreen);
    stack->show();
}

void MainWindow::setupWindow(){
    QObject::connect(sensors->backButton, &QPushButton::clicked, [=] { setWindow(home); });
    QObject::connect(home->sensorsButton, &QPushButton::clicked, [=] { setWindow(sensors); });
    QObject::connect(home->exitButton, &QPushButton::clicked, [=] { stack->close(); });
    //QObject::connect(home->exitButton, &QPushButton::clicked, QCoreApplication::instance(), &QCoreApplication::quit );
    //QObject::connect(stack, &QObject::destroyed, QCoreApplication::instance(), &QCoreApplication::quit);
}

void MainWindow::setWindow(QWidget *w){
    stack->setCurrentWidget(w);
}

MainWindow::~MainWindow()
{
    delete stack;
    delete home;
    delete sensors;
}
