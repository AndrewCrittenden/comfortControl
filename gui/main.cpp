#include "mainwindow.h"
#include <QApplication>
#include <QGuiApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setupWindow();    
    return a.exec();
}
