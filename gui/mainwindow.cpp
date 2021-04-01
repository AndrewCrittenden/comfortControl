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
#include <QtConcurrent/QtConcurrent>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "TripleBuffer.h"
#include "servercomfort.h"

using namespace std;

const int MAX_IPC_CHAR = 100;
const int REFRESH_MEASUREMENTS = 1000; // units milliseconds
const float initial_air_vol = 0.015; // I don't have the actual length/width/height handy right now
const float initial_max_cool = -50;
const float initial_max_heat = 50;
const float initial_sample_time = 15000;

//MainWindow is the controller used to switch between windows and close the application
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), controller(74*initial_air_vol*1.8, 1.1111*initial_air_vol*1.8, 740*initial_air_vol*1.8, initial_max_cool, initial_max_heat, initial_sample_time)
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
    QObject::connect(settings->authenticateButton, &QPushButton::clicked, [=] { server.authenticate = true; });
    QObject::connect(settings->clearNodeButton, &QPushButton::clicked, [=] { server.clearNode = true; });
    server.gatherFrequency = 20000;
    QObject::connect(settings->gatherFreqSlider, &QSlider::valueChanged, this, &MainWindow::updateGatherFreq);
    //Refresh measurement's from serverCOMFORT
    QObject::connect(&server, &serverCOMFORT::inData_receivedChanged, this, &MainWindow::refreshMeasurements);
    //QObject::connect(&server, &serverCOMFORT::sensorsReadyChanged, this, &MainWindow::refreshMeasurements);

    //QTimer *sensorTimer = new QTimer(this);
    //QObject::connect(sensorTimer, &QTimer::timeout, this, &MainWindow::refreshMeasurements);
    //sensorTimer->start(REFRESH_MEASUREMENTS);
    //IPC communication with comfortAnalysis.py
    QFileSystemWatcher *IPCrecieveTrigger = new QFileSystemWatcher;
    IPCrecieveTrigger->addPath("/home/pi/WA/comfortControl/fifo/comfortToGui.fifo");
    QObject::connect(IPCrecieveTrigger, &QFileSystemWatcher::fileChanged, this, &MainWindow::IPCRecieveComfort);
    QObject::connect(home->tempDial, &QDial::valueChanged, this, &MainWindow::IPCSendComfort);
    IPCSendComfort(0);
    //Start serverCOMFORT thread
    QFuture<void> future = QtConcurrent::run([this] {
        server.serverOperation();
    });
    controller.setInitial(g_indoorTemp,g_setpoint_temperature);
    QFile data("/home/pi/WA/comfortControl/gui/data.csv");
    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream output(&data);
        output << "Timestamp, Indoor Temperature (F), Outdoor Temperature (F), Relative Humidity (%), Globe Temperature(F), Occupancy, PMV, Setpoint Temperature (F), Desired Temperature (F), Activity Level, Appliance Wattage (W)\n";
    }
    data.close();
}

void MainWindow::updateGatherFreq(int f){
    QTime t(0, 0, 0);
    t = t.addSecs(f);
    settings->gatherFreqDsp->display(t.toString("m:ss"));
    server.gatherFrequency = f*1000;
}

void MainWindow::refreshMeasurements(){
    dataIn newData = server.buffIn.GetPublicBuffer();
    server.inData_received = false;
    qDebug("Recieving from wireless");
    qDebug() << "Server indoor is " << newData.indoor;
    qDebug() << "Server outdoor is " << newData.outdoor;
    qDebug() << "Server relHumidity is " << newData.relHumidity;
    qDebug() << "Server globe is " << newData.globe;
    qDebug() << "Server occupancy is " << newData.occupancy;
    g_indoorTemp = newData.indoor;
    g_outdoorTemp = newData.outdoor;
    g_relHumidity = newData.relHumidity;
    g_globeTemp = newData.globe;
    g_occupancy = newData.occupancy;
    qDebug() << g_setpoint_temperature << g_indoorTemp << g_outdoorTemp << g_relHumidity << g_globeTemp << g_occupancy << g_activityLevel.c_str();
    sensors->indoorTemp->display(g_indoorTemp);
    sensors->outdoorTemp->display(g_outdoorTemp);
    sensors->relHumidity->display(g_relHumidity);
    sensors->globeTemp->display(g_globeTemp);
    sensors->heatCoolOutput->display(g_heatCoolOutput);
    if (g_occupancy) {
           sensors->occupancy->setText("Occupied");
    }
       else {
           sensors->occupancy->setText("Vacant");
    }
    if(server.sensorsReady){
        IPCSendComfort(g_desiredTemp);
        controller.setSetpoint(g_setpoint_temperature);
        controller.setCurrentTemperature(newData.indoor);
        g_heatCoolOutput = controller.forceUpdate();
        qDebug() << g_heatCoolOutput;
        //Write data to csv file
        QDateTime now = QDateTime::currentDateTimeUtc();
        QFile data("/home/pi/WA/comfortControl/gui/data.csv");
        if (data.open(QFile::WriteOnly | QFile::Truncate | QIODevice::Append)) {
            QTextStream output(&data);
            output << now.toString() << "," << g_indoorTemp << "," << g_outdoorTemp << "," << g_relHumidity << "," << g_globeTemp << "," << g_occupancy << "," << g_pmv << "," << g_setpoint_temperature << "," << g_desiredTemp << "," << g_activityLevel.c_str() << "," << g_heatCoolOutput << "\n";
        }
        data.close();
    }

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
    sensors->setpointTemp->display(g_setpoint_temperature);
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
