#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QWidget>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QLCDNumber>
#include <QTime>

class settingswindow : public QWidget
{
public:
    explicit settingswindow(QWidget *parent = nullptr);
    ~settingswindow();
    QPushButton *backButton;
    QSlider *gatherFreqSlider;
    QLCDNumber *gatherFreqDsp;

private:
    QGridLayout *layout;
    QLabel *gatherFreqLbl;
};

#endif // SETTINGSWINDOW_H
