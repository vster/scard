TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    pcscwrap.c \
    smacgol11.cpp

HEADERS += \
    pcscwrap.h

INCLUDEPATH += \
    /usr/local/include/PCSC

LIBS += \
    -L/usr/local/lib -lpcsclite
