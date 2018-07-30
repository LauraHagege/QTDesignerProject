#-------------------------------------------------
#
# Project created by QtCreator 2018-07-02T09:45:06
#
#-------------------------------------------------

QT       += core gui
CONFIG -= x86_64

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtDesignerProject
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



INCLUDEPATH += "C:\Program Files\DCMTK\include"

QMAKE_LIBDIR += "C:\Program Files\DCMTK\lib"
win32:LIBS += -lAdvapi32 -lofstd -loflog -ldcmdata -ldcmnet -ldcmimage -ldcmimgle -lws2_32 -lnetapi32 -lwsock32 -liphlpapi


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    dialog.cpp \
    customgraphicsscene.cpp

HEADERS += \
        mainwindow.h \
    dialog.h \
    customgraphicsscene.h

FORMS += \
        mainwindow.ui \
    dialog.ui
