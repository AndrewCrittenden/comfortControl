#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QWidget>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>

class settingswindow : public QWidget
{
public:
    explicit settingswindow(QWidget *parent = nullptr);
    ~settingswindow();
    QPushButton *backButton;
    QPushButton *authenticateButton;
    QPushButton *clearNodeButton;
    QSlider *gatherFreqSlider;

private:
    QGridLayout *layout;
    QLabel *gatherFreqLbl;
};

#endif // SETTINGSWINDOW_H
