#include "qtdream.h"

#include <qtdream-gui/log.h>

#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("QDream");

    QtDream w;
    w.show();

    //set default pattern for logging
    log4cpp::PatternLayout * pLayout = new log4cpp::PatternLayout();
    pLayout->setConversionPattern("%d{%Y-%m-%d %H:%M:%S,%l} %p:  %m%n");

    /*Setting up Appender, layout and Category*/
    log4cpp::Appender *appender = new log4cpp::FileAppender("FileAppender", "qtdream-gui.log");

    appender->setLayout(pLayout);
    LogCategory.setAppender(appender);
    LogCategory.setPriority(log4cpp::Priority::DEBUG);

    LOG_NOTICE("Started qtdream");

    return a.exec();
}
