!include(../../config.pri):error("base config file (config.pri) not available")

QT       += core network
QXT      += network

TARGET = QtDream-dbclient
TEMPLATE = app

SOURCES += main.cpp \
        dbclient.cpp

HEADERS += dbclient.h \
