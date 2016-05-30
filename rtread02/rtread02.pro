TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    rtread02.cpp \
    pcscwrap.c

HEADERS += \
    pcscwrap.h

INCLUDEPATH += \
    /usr/include/PCSC

LIBS += \
    -L/usr/lib -lpcsclite
