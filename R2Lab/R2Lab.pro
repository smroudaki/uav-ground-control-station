#-------------------------------------------------
#
# Project created by QtCreator 2017-11-08T16:38:44
#
#-------------------------------------------------

QT       += core gui serialport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = R2Lab
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

# opencv
INCLUDEPATH += /usr/local/include/opencv
LIBS += `pkg-config opencv --libs`

# zbar
LIBS += -L/usr/local/lib -lzbar

# dlib
INCLUDEPATH += -I/usr/local/include -I/usr/include/libpng12
LIBS += -L/usr/local/lib -ldlib -lpng12
LIBS += -llapack -lblas
LIBS += -lpng -ljpeg
LIBS += -pthread
CONFIG += link_pkgconfig
PKGCONFIG += x11

SOURCES += \
        main.cpp \
        dialog.cpp \
    qrcode.cpp \
    motion.cpp \
    dlib/all/source.cpp \
    savenewface.cpp \
    ocr.cpp \
    facerec.cpp \
    circlecolorrec.cpp

HEADERS += \
        dialog.h \
    qrcode.h \
    motion.h \
    savenewface.h \
    ocr.h \
    facerec.h \
    circlecolorrec.h

FORMS += \
        dialog.ui \
    savenewface.ui
