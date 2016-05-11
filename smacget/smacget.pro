TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += \
    /usr/include/PCSC

LIBS += \
    -L/usr/lib -lpcsclite

HEADERS += \
    pcscwrap.h

SOURCES += \
    pcscwrap.c \
    smacget.c
