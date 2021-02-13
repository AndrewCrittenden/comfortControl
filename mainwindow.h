#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStackedWidget>
#include <QMainWindow>
#include "sensorswindow.h"
#include "homewindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setupWindow();
    ~MainWindow();
    HomeWindow *home;

private:
    QStackedWidget *stack;
    SensorsWindow *sensors;

signals:
    void exitApp();

public slots:
    void setWindow(QWidget *w);

};

#endif // MAINWINDOW_H
