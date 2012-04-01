
#include <qtdream-server/server.h>

#include <QApplication>

int main(int argc, char ** argv)
{
    QApplication a(argc, argv);
    a.setApplicationName("QtDream-server");

    Server server;

    server.test();

    return a.exec();
}
