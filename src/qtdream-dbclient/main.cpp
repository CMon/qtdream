#include <qtdream-dbclient/dbclient.h>

#include <QApplication>

int main(int argc, char ** argv)
{
    QApplication a(argc, argv);
    a.setApplicationName("QtDream-server");

    DbClient client("localhost");

    return a.exec();
}
