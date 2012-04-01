!include(../../config.pri):error("base config file (config.pri) not available")

QT       += core gui network xml xmlpatterns

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

# VLC > 1.2 needed
LIBS += -lvlc
macx {
	message("Building for mac")
	INCLUDEPATH += /Applications/VLC.app/Contents/MacOS/include
	LIBS += -L/Applications/VLC.app/Contents/MacOS/lib/
} else {
	message("You need to add the lib path to VLCLib")
}
