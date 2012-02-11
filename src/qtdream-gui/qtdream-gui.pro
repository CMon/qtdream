!include(../../config.pri):error("base config file (config.pri) not available")

QT       += core gui phonon network xml xmlpatterns

TARGET = QtDream
TEMPLATE = app

SOURCES += main.cpp\
        qtdream.cpp \
    dreamboxapi.cpp \
    mediavideowidget.cpp \
    configurationdialog.cpp \
    managementserver.cpp \
    dreamboxmanagementwidget.cpp

HEADERS  += qtdream.h \
    dreamboxapi.h \
    mediavideowidget.h \
    configurationdialog.h \
    managementserver.h \
    log.h \
    dreamboxmanagementwidget.h

FORMS    += qtdream.ui \
    dreamboxmanagementwidget.ui \
    configurationdialog.ui




