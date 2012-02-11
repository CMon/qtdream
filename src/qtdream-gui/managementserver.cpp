#include "managementserver.h"

#include <qtdream-gui/log.h>

#include <QtNetwork>

ManagementServer::ManagementServer(QObject *parent) :
    QObject(parent)
{
    startServer();

    connect(tcpServer_, SIGNAL(newConnection()), this, SLOT(registerNewBox()));
}

ManagementServer::~ManagementServer()
{
    if (tcpServer_) delete tcpServer_;
    foreach (const QString & id, boxes_.keys()) {
        if (boxes_[id]) delete boxes_[id];
    }
}

void ManagementServer::startServer()
 {
    QSettings settings(QSettings::UserScope, QLatin1String("Twist"));
    settings.beginGroup(QLatin1String("Server"));
    quint16 port = settings.value(QLatin1String("ListenPort"), 4324).toUInt();
    settings.endGroup();

    QHostAddress bindAddress = QHostAddress(QHostAddress::LocalHost);
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address())
        {
            bindAddress = ipAddressesList.at(i);
            break;
        }
    }

    tcpServer_ = new QTcpServer(this);
    if (!tcpServer_->listen(bindAddress, port)) {
        lastErrorMsg_ = "Unable to start the server: %1." + tcpServer_->errorString();
        LOG_FATAL(lastErrorMsg_);
        return;
    }
    LOG_DEBUG("The server is running on %1:%2", bindAddress.toString(), port);
}

void ManagementServer::registerNewBox()
{
    QTcpSocket * clientConnection = tcpServer_->nextPendingConnection();
    connect(clientConnection, SIGNAL(disconnected()), this, SLOT(disconnectLater()));
    clientConnection->waitForReadyRead();
    const QByteArray id = clientConnection->readAll();

    if (id.isEmpty()) {
        LOG_ALERT("HackAttack invalid infos from %1", clientConnection->peerAddress().toString());
        clientConnection->disconnect();
        return;
    }

    const QString idStr = QString(id);
    boxes_[idStr] = clientConnection;
    LOG_DEBUG("new Box registered ID: %1", idStr);
    emit newBox(idStr);
}

QString ManagementServer::sendCommand(const QString & boxId, const QByteArray & command) const
{
    if (!boxes_.contains(boxId)) {
        LOG_WARN("box id(%1) is not connected", boxId);
        return QString();
    }

    boxes_[boxId]->write(command);
    boxes_[boxId]->waitForReadyRead();

    return QString(boxes_[boxId]->readAll());
}

void ManagementServer::disconnectLater()
{
    QTcpSocket * clientConnection = static_cast<QTcpSocket*>(sender());
    if (!clientConnection) return;

    foreach (const QString & id, boxes_.keys()) {
        if (boxes_[id] == clientConnection) {
            boxes_.remove(id);
            break;
        }
    }

    clientConnection->deleteLater();
}

QString ManagementServer::createSSHTunnel(const QString & boxId, int localPort) const
{
    if (boxId.isEmpty() || localPort <= 0) {
        LOG_ERROR("Box id (%1) or localPort (%2) invalid ", boxId, localPort);
        return QString ();
    }
    return sendCommand(boxId, "CREATE SSH|" + QByteArray::number(localPort));
}

