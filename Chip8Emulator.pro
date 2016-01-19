#-------------------------------------------------
#
# Project created by QtCreator 2016-01-11T13:57:40
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Chip8Emulator
TEMPLATE = app

SOURCES += main.cpp\
        gui.cpp \
    chip8.cpp

HEADERS  += gui.h \
    chip8.h

RESOURCES += \
    resources.qrc
