#-------------------------------------------------
#
# Project created by QtCreator 2017-12-18T12:53:07
#
#-------------------------------------------------

QT       += core gui widgets network concurrent xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RaspTest
TEMPLATE = app

target.path = /home/pi
INSTALLS += target

CODECFORSRC = utf-8
CONFIG += C++11

INCLUDEPATH += $$PWD/3rd/JQHttpServer/library/JQLibrary/include \
               $$PWD/3rd/RPi-RFID

include (./3rd/3rd.pri)
include (./form/form.pri)
include (./threadProxy/threadProxy.pri)
include (./main/main.pri)
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


