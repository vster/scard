TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    pcscwrap.c \
    smacdig.c

HEADERS += \
    pcscwrap.h

INCLUDEPATH += \
    /usr/include/PCSC

LIBS += \
    -L/usr/lib -lpcsclite
