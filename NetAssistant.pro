#-------------------------------------------------
#
# Project created by QtCreator 2017-09-09T21:52:49
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NetAssistant
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += main.cpp\
        mainwindow.cpp \
    udp.cpp \
    app.cpp \
    commonhelper.cpp \
    tcp.cpp


HEADERS  += mainwindow.h \
    udp.h \
    app.h \
    commonhelper.h \
    myhelper.h \
    tcp.h


FORMS    += mainwindow.ui

QT += network
