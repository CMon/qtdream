#include "qtdream.h"

#include <qtdream-gui/log.h>

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <qdebug.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("QtDream-gui");

    QtDream w;
    w.show();

    //set default pattern for logging
    log4cpp::PatternLayout * pLayout = new log4cpp::PatternLayout();
    pLayout->setConversionPattern("%d{%Y-%m-%d %H:%M:%S,%l} %6p: %m%n");

    const QString logPath = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
    if(! QDir(logPath).exists()) {
        QDir().mkpath(logPath);
    }

    /*Setting up Appender, layout and Category*/
    const QString logFile = logPath + "/qtdream-gui.log";
    log4cpp::Appender * appender = new log4cpp::FileAppender("FileAppender", logFile.toStdString());
    appender->setLayout(pLayout);
    log4cpp::Appender * consoleAppender = new log4cpp::OstreamAppender("OstreamAppender",&std::cout);
    consoleAppender->setLayout(pLayout);

    LogCategory.setAppender(appender);
    LogCategory.setAppender(consoleAppender);
    LogCategory.setPriority(log4cpp::Priority::DEBUG);

    qDebug() << "Logging to " + logFile;

    LOG_NOTICE("Started qtdream");

    return a.exec();
}
