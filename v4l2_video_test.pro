TEMPLATE = app
CONFIG += console-stdc11
CONFIG -= app_bundle
CONFIG -= qt

TARGET = test
SOURCES += main.c \
    v4l2.c
TARGET = test

HEADERS += \
    v4l2.h

INCLUDEPATH += \
/usr/include \
/usr/local/include \
