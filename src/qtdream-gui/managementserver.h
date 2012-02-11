#pragma once

#include <QHash>
#include <QObject>
#include <QStringList>

class QTcpServer;
class QTcpSocket;

class ManagementServer : public QObject
{
    Q_OBJECT
public:
    ManagementServer(QObject *parent = 0);
    ~ManagementServer();

    QStringList getConnectedBoxIds() const { return boxes_.keys();}

    QString createSSHTunnel(const QString & boxId, int localPort) const;

signals:
    void newBox(const QString & id);

private slots:
    void registerNewBox();
    void disconnectLater();

private:
    void startServer();
    QString sendCommand(const QString & boxId, const QByteArray & command) const;

private:
    QHash<QString, QTcpSocket *> boxes_;
    QString                      lastErrorMsg_;
    QTcpServer *                 tcpServer_;
};

