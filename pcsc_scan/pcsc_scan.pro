TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    pcsc_scan.c

INCLUDEPATH += \
    /usr/include/PCSC

LIBS += \
    -L/usr/lib -lpcsclite
