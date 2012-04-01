#pragma once

#include <QHostInfo>

class Server : QObject
{
    Q_OBJECT

public:
    Server();

    void test();

signals:
    void printClientInfos();

private:
//    QxtRPCPeer server;
};
