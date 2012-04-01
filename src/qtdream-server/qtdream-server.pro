!include(../../config.pri):error("base config file (config.pri) not available")

QT       += core network
QXT      += network

TARGET = QtDream-server
TEMPLATE = app

SOURCES += main.cpp \
        server.cpp

HEADERS += server.h \
