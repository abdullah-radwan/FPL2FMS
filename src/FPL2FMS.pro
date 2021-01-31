QT += core gui widgets autoupdaterwidgets

CONFIG += c++17

SOURCES += \
    converter.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    converter.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

RESOURCES += ../res/res.qrc

INCLUDEPATH += ../lib/include

LIBS += -L../lib -lpugixml

VERSION = 1.1

win32
{
    RC_ICONS = ../res/icon.ico
    QMAKE_TARGET_DESCRIPTION = Convert Garmin FPL files to X-Plane 11 FMS files.
    QMAKE_TARGET_COPYRIGHT = Copyright 2020-2021, Abdullah Radwan
}
