TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    smacgol03.cpp \
    pcscwrap.c

HEADERS += \
    pcscwrap.h

INCLUDEPATH += \
    /usr/local/include/PCSC

LIBS += \
    -L/usr/local/lib -lpcsclite
