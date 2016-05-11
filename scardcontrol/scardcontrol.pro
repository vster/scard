TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    PCSCv2part10.c \
    scardcontrol.c

HEADERS += \
    PCSCv2part10.h

INCLUDEPATH += \
    /usr/local/include/PCSC

LIBS += \
    -L/usr/local/lib -lpcsclite
