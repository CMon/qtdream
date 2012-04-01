#include "qtdream.h"

#include <qtdream-gui/log.h>

#include <QApplication>
#include <QDir>
#include <qdebug.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("QtDream-gui");

    QtDream w;
    w.show();

    LOG_NOTICE("Started " + QApplication::applicationName());

    return a.exec();
}
