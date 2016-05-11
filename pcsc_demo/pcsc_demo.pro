TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    pcsc_demo.c

INCLUDEPATH += \
    /usr/include/PCSC

LIBS += \
    -L/usr/lib -lpcsclite
