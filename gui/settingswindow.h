#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QWidget>
#include <QLayout>
#include <QPushButton>
#include <QLabel>

class settingswindow : public QWidget
{
public:
    explicit settingswindow(QWidget *parent = nullptr);
    ~settingswindow();
    QPushButton *backButton;

private:
    QGridLayout *layout;
    QPushButton *authenticateButton;
};

#endif // SETTINGSWINDOW_H
