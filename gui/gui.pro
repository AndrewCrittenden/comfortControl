#-------------------------------------------------
#
# Project created by QtCreator 2021-02-11T16:41:38
#
#-------------------------------------------------

QT       += core gui
QT_QUICK_CONTROLS_STYLE=material ./app

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gui
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

QMAKE_CXXFLAGS += -g

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    sensorswindow.cpp \
    homewindow.cpp \
    settingswindow.cpp \
    ControlAlgorithm.cpp \
    PID.c \
    nodecomfort.cpp \
    servercomfort.cpp \
    TripleBuffer.cpp

HEADERS += \
        mainwindow.h \
    sensorswindow.h \
    homewindow.h \
    measurements.h \
    settingswindow.h \
    ControlAlgorithm.h \
    PID.h \
    nodecomfort.h \
    servercomfort.h \
    TripleBuffer.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix:!macx: LIBS += -L$$PWD/cryptopp850/ -lcryptopp

INCLUDEPATH += $$PWD/cryptopp850
DEPENDPATH += $$PWD/cryptopp850

CONFIG += no_keywords
unix:!macx: LIBS += -L/usr/include/python3.7m -lpython3.7m
INCLUDEPATH += /usr/include/python3.7m
DEPENDPATH += /usr/include/python3.7m

unix:!macx: PRE_TARGETDEPS += $$PWD/cryptopp850/libcryptopp.a
