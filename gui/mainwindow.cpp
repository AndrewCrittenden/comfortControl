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
const float initial_max_cool = -5;
const float initial_max_heat = 5;
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
    qDebug() << "Please wait, Setting up comfortAnalysis.py";
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(\".\")");
    pName = PyUnicode_DecodeFSDefault("comfortAnalysis");
    pModule = PyImport_Import(pName);
    pFunc = PyObject_GetAttrString(pModule,"comfortAnalysis");
    updatePMV();
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
    QObject::connect(&server, &serverCOMFORT::statusIndoorChanged, this, &MainWindow::indoorStatus);
    QObject::connect(&server, &serverCOMFORT::statusOutdoorChanged, this, &MainWindow::outdoorStatus);
    QObject::connect(&server, &serverCOMFORT::statusRelHumChanged, this, &MainWindow::relHumStatus);
    QObject::connect(&server, &serverCOMFORT::statusGlobeChanged, this, &MainWindow::globeStatus);
    QObject::connect(&server, &serverCOMFORT::statusOccupancyChanged, this, &MainWindow::occupancyStatus);

    //QTimer *sensorTimer = new QTimer(this);
    //QObject::connect(sensorTimer, &QTimer::timeout, this, &MainWindow::updatePMV);
    //sensorTimer->start(REFRESH_MEASUREMENTS);

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

void MainWindow::updatePMV(){
    PyObject *pArgs, *pValue;    
    int numArgs = 5;
    pArgs = PyTuple_New(numArgs);
    //Define arguements to pass to python code
    pValue = PyLong_FromDouble(g_indoorTemp);
    PyTuple_SetItem(pArgs, 0, pValue); //indoorTemp
    pValue = PyLong_FromDouble(g_globeTemp);
    PyTuple_SetItem(pArgs, 1, pValue); //globeTemp
    pValue = PyLong_FromDouble(g_relHumidity);
    PyTuple_SetItem(pArgs, 2, pValue); //relativeHumidity
    pValue = PyLong_FromDouble(g_outdoorTemp);
    PyTuple_SetItem(pArgs, 3, pValue); //outdoorTemp
    pValue = PyLong_FromLong(g_occupancy);
    PyTuple_SetItem(pArgs, 4, pValue); //occupancy
    pValue = PyObject_CallObject(pFunc, pArgs);
    double pyPmv, pySetpoint;
    PyArg_ParseTuple(pValue,"d|d",&pyPmv,&pySetpoint);
    //printf("Result of call: %f, %f\n",pyPmv,pySetpoint);
    g_pmv = pyPmv;
    g_setpoint_temperature = pySetpoint;
    sensors->pmv->display(g_pmv);
    sensors->setpointTemp->display(g_setpoint_temperature);
}

void MainWindow::refreshMeasurements(){
    dataIn newData = server.buffIn.GetPublicBuffer();
    server.inData_received = false;
    //qDebug("Recieving from wireless");
    //qDebug() << "Server indoor is " << newData.indoor;
    //qDebug() << "Server outdoor is " << newData.outdoor;
    //qDebug() << "Server relHumidity is " << newData.relHumidity;
    //qDebug() << "Server globe is " << newData.globe;
    //qDebug() << "Server occupancy is " << newData.occupancy;
    g_indoorTemp = newData.indoor;
    g_outdoorTemp = newData.outdoor;
    g_relHumidity = newData.relHumidity;
    g_globeTemp = newData.globe*9/5+32; //convert to farhenheit
    g_occupancy = newData.occupancy;
    //qDebug() << g_setpoint_temperature << g_indoorTemp << g_outdoorTemp << g_relHumidity << g_globeTemp << g_occupancy << g_activityLevel.c_str();
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
        updatePMV();
        controller.setSetpoint(g_setpoint_temperature);
        controller.setCurrentTemperature(newData.indoor);
        g_heatCoolOutput = controller.forceUpdate();
        //qDebug() << g_heatCoolOutput;
        dataOut toSend;
        toSend.output = g_heatCoolOutput;
        server.buffOut.SetPrivateBuffer(toSend);
        server.buffOut.RotatePrivateBuffer();
        server.outData_toSend = true;
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

void MainWindow::indoorStatus(bool value){
    if(value){
        sensors->indoorTempSatusLbl->setText("Connected");
    }
    else{
        sensors->indoorTempSatusLbl->setText("Disconnected");
    }
}
void MainWindow::outdoorStatus(bool value){
    if(value){
        sensors->outdoorTempSatusLbl->setText("Connected");
    }
    else{
        sensors->outdoorTempSatusLbl->setText("Disconnected");
    }
}
void MainWindow::relHumStatus(bool value){
    if(value){
        sensors->relHumiditySatusLbl->setText("Connected");
    }
    else{
        sensors->relHumiditySatusLbl->setText("Disconnected");
    }
}
void MainWindow::globeStatus(bool value){
    if(value){
        sensors->globeTempSatusLbl->setText("Connected");
    }
    else{
        sensors->globeTempSatusLbl->setText("Disconnected");
    }
}
void MainWindow::occupancyStatus(bool value){
    if(value){
        sensors->occupancySatusLbl->setText("Connected");
    }
    else{
        sensors->occupancySatusLbl->setText("Disconnected");
    }
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
