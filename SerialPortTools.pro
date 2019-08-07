#-------------------------------------------------
#
# Project created by QtCreator 2011-12-22T22:11:03
#
#-------------------------------------------------

QT       += core gui widgets multimedia

include(./src/qextserialport.pri)

TARGET = SerialPortTools
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS += mainwindow.h \
        mhelper.h

FORMS += mainwindow.ui

RESOURCES += \
    resource.qrc

win32{
    message("-->>Windows")
    RC_FILE=app.rc
}
unix:!macx {
    message("-->>Linux")
    RC_FILE=app.rc
}

macx: {
    message("-->>macOS")
    ICON = logo.icns
}

CONFIG += warn_off
