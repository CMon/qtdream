#include "dbclient.h"

#include <QString>

#include <qdebug.h>

DbClient::DbClient(const QString & serverHost)
{
    hostInfo.fromName(serverHost);
//    client.connect (hostInfo.addresses().first(), 4711);

//    client.attachSlot(SIGNAL(printClientInfos()), this, SLOT(printClientInfos()));
}

void DbClient::printClientInfos()
{
    qDebug () << "Hello";
}
