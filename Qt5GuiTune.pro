#-------------------------------------------------
#
# Project created by QtCreator 2018-08-22T07:19:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Qt5GuiTune
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


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    audiodialog.cpp \
    logview.cpp \
    osziview.cpp \
    scaledialog.cpp \
    soundinput.cpp \
    tuningsdialog.cpp

HEADERS += \
        mainwindow.h \
    audiodialog.h \
    logview.h \
    osziview.h \
    resources.h \
    scaledialog.h \
    soundinput.h \
    tuningsdialog.h

FORMS += \
        mainwindow.ui \
    audiodialog.ui \
    scaledialog.ui \
    tuningsdialog.ui

unix:!macx: LIBS += -lasound

unix:!macx: LIBS += -lfftw3

DISTFILES += \
    images/down_arrow.png \
    images/up_arrow.png \
    qss/spinbox.qss
