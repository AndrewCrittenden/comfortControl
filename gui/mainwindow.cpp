#include "mainwindow.h"
#include <QCoreApplication>
#include <QFileSystemWatcher>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <QThread>
#include <QTimer>
#include "measurements.h"
using namespace std;

const int MAX_IPC_CHAR = 100;
const int REFRESH_MEASUREMENTS = 1000; // units milliseconds

//MainWindow is the controller used to switch between windows and close the application
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    stack = new QStackedWidget;
    home = new HomeWindow;
    sensors = new SensorsWindow;
    settings = new settingswindow;
    stack->addWidget(home);
    stack->addWidget(sensors);
    stack->addWidget(settings);
    stack->setCurrentWidget(home);
    //stack->setWindowState(Qt::WindowFullScreen);
    stack->show();
}

void MainWindow::setupWindow(){
    QObject::connect(sensors->backButton, &QPushButton::clicked, [=] { setWindow(home); });
    QObject::connect(settings->backButton, &QPushButton::clicked, [=] { setWindow(home); });
    QObject::connect(home->sensorsButton, &QPushButton::clicked, [=] { setWindow(sensors); });
    QObject::connect(home->settingsButton, &QPushButton::clicked, [=] { setWindow(settings); });
    QObject::connect(home->exitButton, &QPushButton::clicked, [=] { stack->close(); });
    QTimer *sensorTimer = new QTimer(this);
    QObject::connect(sensorTimer, &QTimer::timeout, this, &MainWindow::refreshMeasurements);
    sensorTimer->start(REFRESH_MEASUREMENTS);
    //IPC communication with comfortAnalysis.py
    QFileSystemWatcher *IPCrecieveTrigger = new QFileSystemWatcher;
    IPCrecieveTrigger->addPath("/home/pi/WA/comfortControl/fifo/comfortToGui.fifo");
    QObject::connect(IPCrecieveTrigger, &QFileSystemWatcher::fileChanged, this, &MainWindow::IPCRecieveComfort);
    QObject::connect(home->tempDial, &QDial::valueChanged, this, &MainWindow::IPCSendComfort);
    IPCSendComfort(0);
}

void MainWindow::refreshMeasurements(){
    //qDebug("Recieving from wireless");
    //TODO fetch wireless data from Tyler
    g_indoorTemp = (80 - 60) * ((((float) rand()) / (float) RAND_MAX)) + 60;
    g_outdoorTemp = (80 - 60) * ((((float) rand()) / (float) RAND_MAX)) + 60;
    g_relHumidity = (1 - 0) * ((((float) rand()) / (float) RAND_MAX)) + 0;
    g_globeTemp = (80 - 60) * ((((float) rand()) / (float) RAND_MAX)) + 60;
    bool occupancy_options[] = {true, false};
    g_occupancy = occupancy_options[rand()%2];
    string activity_options[] = {"resting","moderately active","active"};
    g_activityLevel = activity_options[rand()%3];
    qDebug() << g_indoorTemp << g_outdoorTemp << g_relHumidity << g_globeTemp << g_occupancy << g_activityLevel.c_str();
    sensors->indoorTemp->display(g_indoorTemp);
    sensors->outdoorTemp->display(g_outdoorTemp);
    sensors->relHumidity->display(g_relHumidity);
    sensors->globeTemp->display(g_globeTemp);
    if (g_occupancy) {
           sensors->occupancy->setText("Occupied");
    }
       else {
           sensors->occupancy->setText("Vacant");
    }
    IPCSendComfort(g_desiredTemp);
}

void MainWindow::IPCRecieveComfort(){
    const char *fifo = "/home/pi/WA/comfortControl/fifo/comfortToGui.fifo";
    mkfifo(fifo, 0666);
    int fd = open(fifo,O_RDONLY);
    char buf[MAX_IPC_CHAR];
    read(fd,buf,MAX_IPC_CHAR);
    QString str = QString(buf);
    QStringList list = str.split(',');
    //qDebug() << list;
    g_pmv = list[0].toFloat();
    g_setpoint_temperature = list[1].toFloat();
    sensors->pmv->display(g_pmv);
    g_isComfortable = true;

}

void MainWindow::IPCSendComfort(int dt){
    g_desiredTemp = dt;
    const char *fifo = "/home/pi/WA/comfortControl/fifo/guiToComfort.fifo";
    mkfifo(fifo, 0666);    
    string dtStr = to_string(g_desiredTemp)+","+
            g_activityLevel+","+
            to_string(g_occupancy)+","+
            to_string(g_globeTemp)+","+
            to_string(g_relHumidity)+","+
            to_string(g_outdoorTemp)+","+
            to_string(g_indoorTemp)+",\n";
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
