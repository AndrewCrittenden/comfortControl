#include "mainwindow.h"
#include <QCoreApplication>
//#include <QTimer>
#include <QFileSystemWatcher>

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

const int MAX_IPC_CHAR = 100;

//MainWindow is the controller used to switch between windows and close the application
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    stack = new QStackedWidget;
    home = new HomeWindow;
    sensors = new SensorsWindow;
    stack->addWidget(home);
    stack->addWidget(sensors);
    stack->setCurrentWidget(home);
    stack->setWindowState(Qt::WindowFullScreen);
    stack->show();
}

void MainWindow::setupWindow(){
    QObject::connect(sensors->backButton, &QPushButton::clicked, [=] { setWindow(home); });
    QObject::connect(home->sensorsButton, &QPushButton::clicked, [=] { setWindow(sensors); });
    QObject::connect(home->exitButton, &QPushButton::clicked, [=] { stack->close(); });
    //QObject::connect(home->exitButton, &QPushButton::clicked, QCoreApplication::instance(), &QCoreApplication::quit );
    //QObject::connect(stack, &QObject::destroyed, QCoreApplication::instance(), &QCoreApplication::quit);
    //IPCRecieveComfort();
    //QTimer *IPCtimer = new QTimer(this);
    //QTimer *IPCtimer2 = new QTimer(this);
    //QObject::connect(IPCtimer, &QTimer::timeout, this, &MainWindow::IPCSendComfort);
    //QObject::connect(IPCtimer, &QTimer::timeout, this, &MainWindow::IPCRecieveComfort);
    //IPCtimer->start(1500);
    //IPCtimer2->start(3000);

    QFileSystemWatcher *IPCrecieveTrigger = new QFileSystemWatcher;
    //QFileSystemWatcher *IPCsendTrigger = new QFileSystemWatcher;
    IPCrecieveTrigger->addPath("/home/pi/WA/comfortControl/fifo/comfortToGui.fifo");
    //IPCsendTrigger->addPath("/home/pi/WA/comfortControl/fifo/guiToComfort.fifo");
    QObject::connect(IPCrecieveTrigger, &QFileSystemWatcher::fileChanged, this, &MainWindow::IPCRecieveComfort);
    QObject::connect(home->tempDial, &QDial::valueChanged, this, &MainWindow::IPCSendComfort);
    //QObject::connect(IPCrecieveTrigger, &QFileSystemWatcher::fileChanged, this, &MainWindow::IPCSendComfort);
    //system("python3 /home/pi/WA/comfortControl/comfortCode/IPCSample.py");
    IPCSendComfort(0);
}

void MainWindow::IPCRecieveComfort(){
    qDebug("Recieving from comfort");
    const char *fifo = "/home/pi/WA/comfortControl/fifo/comfortToGui.fifo";
    mkfifo(fifo, 0666);
    int fd = open(fifo,O_RDONLY);
    char buf[MAX_IPC_CHAR];
    read(fd,buf,MAX_IPC_CHAR);
    QString str = QString(buf);
    QStringList list = str.split(',');
    //qDebug() << list;
    sensors->indoorTemp->display(list[0].toDouble());
    sensors->outdoorTemp->display(list[1].toDouble());
    sensors->absHumidity->display(list[2].toDouble());
    sensors->relHumidity->display(list[3].toDouble());
    sensors->globeTemp->display(list[4].toDouble());
    if (list[5] == "True") {
        sensors->occupancy->setText("Occupied"); }
    else {
        sensors->occupancy->setText("Vacant"); }
    sensors->pmv->display(list[6].toDouble());
}

void MainWindow::IPCSendComfort(int dt){
    const char *fifo = "/home/pi/WA/comfortControl/fifo/guiToComfort.fifo";
    mkfifo(fifo, 0666);
    string dtStr = to_string(dt)+",resting,\n";
    int fd = open(fifo, O_NONBLOCK|O_RDWR);
    write(fd,dtStr.c_str(),dtStr.length()+1);
    ::close(fd);
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
