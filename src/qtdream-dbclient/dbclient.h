#pragma once

#include <QHostInfo>
//#include <QxtRPCPeer>

class QString;

class DbClient : QObject
{
    Q_OBJECT

public:
    DbClient(const QString & serverHost);

private slots:
    void printClientInfos();

private:
    QHostInfo  hostInfo;
//    QxtRPCPeer client;
};
