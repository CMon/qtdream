#include "dreamboxapi.h"

#include <QBuffer>
#include <QNetworkRequest>
#include <QtXml>
#include <QXmlItem>
#include <QXmlQuery>
#include <QXmlResultItems>

/*
You can find the api description here: http://dream.reichholf.net/wiki/Enigma2:WebInterface
*/

namespace {

QString evaluateItem(QXmlQuery & query, const QString & queryString)
{
    QString data;
    query.setQuery(queryString + "/string()");
    query.evaluateTo(&data);
    return data.trimmed();
}

bool string2Bool(const QString & str)
{
    return str.compare("true", Qt::CaseInsensitive) ? true : false;
}

}


DreamboxApi::DreamboxApi(const QString & host, int port, int streamPort, QObject *parent) :
    QObject(parent),
    manager (new QNetworkAccessManager(this))
{
    requestBase_ = "http://" + host + ":" + QString::number(port) + "/web/";
    streamBase_ = "http://" + host + ":" + QString::number(streamPort) + "/";
}

DreamboxApi::~DreamboxApi ()
{
    delete manager;
}

void DreamboxApi::requestGeneralBoxInfo() const
{
    QNetworkRequest request;
    request.setUrl(QUrl(requestBase_ + "about"));

    QNetworkReply *reply = manager->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(generalBoxInfo()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
}

void DreamboxApi::requestCurrentAudioTracks() const
{
    QNetworkRequest request;
    request.setUrl(QUrl(requestBase_ + "getaudiotracks"));

    QNetworkReply *reply = manager->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(currentAudioTracks()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
}

void DreamboxApi::requestCurrentChannel() const
{
    QNetworkRequest request;
    request.setUrl(QUrl(requestBase_ + "subservices"));

    QNetworkReply *reply = manager->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(currentChannel()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
}

void DreamboxApi::requestStreamingUrl(const QString & serviceReference) const
{
    QNetworkRequest request;
    request.setUrl(QUrl(requestBase_ + "stream.m3u?ref=" + serviceReference));

    QNetworkReply *reply = manager->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(currentStreamingUrl()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
}

void DreamboxApi::requestVolume() const
{
    changeVolume("state");
}

void DreamboxApi::setVolume(int volume) const
{
    changeVolume("set" + QString::number(volume));
}

void DreamboxApi::incVolume() const
{
    changeVolume("up");
}

void DreamboxApi::decVolume() const
{
    changeVolume("down");
}

void DreamboxApi::toggleMute() const
{
    changeVolume("mute");
}

void DreamboxApi::changeVolume(const QString & command) const
{
    QNetworkRequest request;
    request.setUrl(QUrl(requestBase_ + "vol?set=" + command));

    QNetworkReply *reply = manager->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(volumeChanged()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
}

void DreamboxApi::requestBouquets() const
{
    QNetworkRequest request;
    request.setUrl(QUrl(requestBase_ + "getservices"));

    QNetworkReply *reply = manager->get(request);
    reply->setProperty("reference", "");
    connect(reply, SIGNAL(finished()), this, SLOT(newService()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
}

void DreamboxApi::requestSenders(const QString & bouquetReference) const
{
    QNetworkRequest request;
    request.setUrl(QUrl(requestBase_ + "getservices?sRef=" + bouquetReference));

    QNetworkReply *reply = manager->get(request);
    reply->setProperty("reference", bouquetReference);
    connect(reply, SIGNAL(finished()), this, SLOT(newService()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
}

void DreamboxApi::requestSenderDetails(const QString & reference) const
{
    QNetworkRequest request;
    request.setUrl(QUrl(requestBase_ + "epgservice?sRef=" + reference));

    QNetworkReply *reply = manager->get(request);
    reply->setProperty("reference", reference);

    connect(reply, SIGNAL(finished()), this, SLOT(updateSenderDetail()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
}

void DreamboxApi::error(QNetworkReply::NetworkError /*err*/)
{
    emit errorString(static_cast<QNetworkReply*>(sender())->errorString());
}

void DreamboxApi::newService()
{
    QList<Enigma2Service> services;
    QNetworkReply * reply = static_cast<QNetworkReply*>(sender());
    QByteArray answer = reply->readAll();
    QString    reference = reply->property("reference").toString();
    bool isSender = !reference.isEmpty();

    QBuffer ioBuffer;
    ioBuffer.setData(answer);
    ioBuffer.open(QBuffer::ReadOnly);
    ioBuffer.reset();

    QXmlQuery query;
    query.setFocus(&ioBuffer);
    query.setQuery( "/e2servicelist/e2service" );

    QXmlResultItems result;
    query.evaluateTo(&result);
    QXmlItem item(result.next());
    while (!item.isNull()) {
        Enigma2Service tmp;
        query.setFocus(item);

        tmp.reference = evaluateItem(query, "e2servicereference");
        tmp.name = evaluateItem(query, "e2servicename");

        services << tmp;
        item = result.next();
    }

    if (isSender) emit newSenders(reference, services);
    else          emit newBouquets(services);
}

void DreamboxApi::updateSenderDetail()
{
    QList<Enigma2Event> epgs;
    QNetworkReply * reply = static_cast<QNetworkReply*>(sender());
    QByteArray answer = reply->readAll();
    QString    sReference = reply->property("reference").toString();

    QBuffer ioBuffer;
    ioBuffer.setData(answer);
    ioBuffer.open(QBuffer::ReadOnly);
    ioBuffer.reset();

    QXmlQuery query;
    query.setFocus(&ioBuffer);
    query.setQuery( "/e2eventlist/e2event" );

    QXmlResultItems result;
    query.evaluateTo(&result);
    QXmlItem item(result.next());
    while (!item.isNull()) {
        Enigma2Event tmp;
        query.setFocus(item);

        tmp.id                  = evaluateItem(query, "e2eventid").toInt();
        tmp.start               = QDateTime::fromTime_t(evaluateItem(query, "e2eventstart").toUInt());
        tmp.duration            = evaluateItem(query, "e2eventduration").toInt();
        tmp.currentTime         = QDateTime::fromTime_t(evaluateItem(query, "e2eventcurrenttime").toUInt());
        tmp.title               = evaluateItem(query, "e2eventtitle");
        tmp.description         = evaluateItem(query, "e2eventdescription");
        tmp.extendedDescription = evaluateItem(query, "e2eventdescriptionextended");
        tmp.reference           = evaluateItem(query, "e2eventservicereference");
        tmp.serviceName         = evaluateItem(query, "e2eventservicename");

        epgs << tmp;
        item = result.next();
    }

    emit updateEPGs(sReference, epgs);
}


void DreamboxApi::generalBoxInfo()
{
    QList<GeneralBoxInfo> infos;

    QNetworkReply * reply = static_cast<QNetworkReply*>(sender());
    QByteArray answer = reply->readAll();

    QBuffer ioBuffer;
    ioBuffer.setData(answer);
    ioBuffer.open(QBuffer::ReadOnly);
    ioBuffer.reset();

    QXmlQuery query;
    query.setFocus(&ioBuffer);
    query.setQuery( "/e2abouts/e2about" );

    QXmlResultItems result;
    query.evaluateTo(&result);
    QXmlItem item(result.next());
    while (!item.isNull()) {
        GeneralBoxInfo info;

        query.setFocus(item);

        info.enigmaVersion       = evaluateItem(query, "e2enigmaversion");
        info.isDHCPenabled       = string2Bool(evaluateItem(query, "e2landhcp"));
        info.ip                  = evaluateItem(query, "e2lanip");
        info.netmask             = evaluateItem(query, "e2lanmask");
        info.gateway             = evaluateItem(query, "e2langw");
        info.fpVersion           = evaluateItem(query, "e2fpversion");
        info.webInterfaceVersion = evaluateItem(query, "e2webifversion");

        query.setQuery( "/e2tunerinfo/e2nim" );
        QXmlResultItems innerResult;
        query.evaluateTo(&innerResult);
        QXmlItem innerItem(innerResult.next());
        query.setFocus(innerItem);
        while (!innerItem.isNull()) {
            TunerInfo ti;
            ti.name = evaluateItem(query, "name");
            ti.type = evaluateItem(query, "type");

            innerItem = innerResult.next();
            info.tunerInformations << ti;
        }

        query.setFocus(item);
        info.hddModel    = evaluateItem(query, "e2hddinfo/model");;
        info.hddCapacity = evaluateItem(query, "e2hddinfo/capacity");;
        info.hddFree     = evaluateItem(query, "e2hddinfo/free");;

        info.serviceName         = evaluateItem(query, "e2servicename");
        info.serviceProvider     = evaluateItem(query, "e2serviceprovider");
        info.aspect              = evaluateItem(query, "e2serviceaspect");
        info.videoSize           = evaluateItem(query, "e2servicevideosize");
        info.serviceNamespace    = evaluateItem(query, "e2servicenamespace");
        info.vpid                = evaluateItem(query, "e2vpid").toInt();
        info.apid                = evaluateItem(query, "e2apid").toInt();
        info.pcrid               = evaluateItem(query, "e2pcrid").toInt();
        info.pmtpid              = evaluateItem(query, "e2pmtpid").toInt();
        info.txtpid              = evaluateItem(query, "e2txtpid").toInt();
        info.tsid                = evaluateItem(query, "e2tsid").toInt();
        info.onid                = evaluateItem(query, "e2onid").toInt();
        info.sid                 = evaluateItem(query, "e2sid").toInt();

        infos << info;
        item = result.next();
    }

    emit generalBoxInfos(infos);
}

void DreamboxApi::volumeChanged()
{
    QNetworkReply * reply = static_cast<QNetworkReply*>(sender());
    QByteArray answer = reply->readAll();

    QBuffer ioBuffer;
    ioBuffer.setData(answer);
    ioBuffer.open(QBuffer::ReadOnly);
    ioBuffer.reset();

    QXmlQuery query;
    query.setFocus(&ioBuffer);
    query.setQuery( "/e2volume" );

    QXmlResultItems result;
    query.evaluateTo(&result);
    QXmlItem item(result.next());
    while (!item.isNull()) {
        query.setFocus(item);

        bool ok = string2Bool(evaluateItem(query, "e2result"));
        if (!ok) {
            item = result.next();
            continue;
        }

        emit volume(evaluateItem(query, "e2current").toInt());
        emit muted(string2Bool(evaluateItem(query, "e2ismuted")));
        break;
    }
}

void DreamboxApi::currentChannel()
{
    QString reference;

    QNetworkReply * reply = static_cast<QNetworkReply*>(sender());
    QByteArray answer = reply->readAll();

    QBuffer ioBuffer;
    ioBuffer.setData(answer);
    ioBuffer.open(QBuffer::ReadOnly);
    ioBuffer.reset();

    QXmlQuery query;
    query.setFocus(&ioBuffer);
    query.setQuery( "/e2servicelist/e2service" );

    QXmlResultItems result;
    query.evaluateTo(&result);
    QXmlItem item(result.next());
    while (!item.isNull()) {
        query.setFocus(item);

        reference = evaluateItem(query, "e2servicereference");
        break;
    }

    emit currentChannel(reference);
}

void DreamboxApi::currentStreamingUrl()
{
    QString url;

    QNetworkReply * reply = static_cast<QNetworkReply*>(sender());
    QStringList lines = QString(reply->readAll()).split("\n");

    foreach (const QString & line, lines) {
        if (line.startsWith("http://")) {
            url = line.trimmed();
            break;
        }
    }

    if (!url.isEmpty()) emit streamingUrl(url);
}

void DreamboxApi::currentAudioTracks()
{
    QList<AudioTrack> tracks;

    QNetworkReply * reply = static_cast<QNetworkReply*>(sender());
    QByteArray answer = reply->readAll();

    QBuffer ioBuffer;
    ioBuffer.setData(answer);
    ioBuffer.open(QBuffer::ReadOnly);
    ioBuffer.reset();

    QXmlQuery query;
    query.setFocus(&ioBuffer);
    query.setQuery( "/e2audiotracklist/e2audiotrack" );

    QXmlResultItems result;
    query.evaluateTo(&result);
    QXmlItem item(result.next());
    while (!item.isNull()) {
        AudioTrack track;
        query.setFocus(item);

        track.description = evaluateItem(query, "e2servicereference");
        track.id          = evaluateItem(query, "e2servicereference").toInt();
        track.pId         = evaluateItem(query, "e2servicereference").toInt();
        track.active      = string2Bool(evaluateItem(query, "e2servicereference"));

        tracks << track;

        item = result.next();
    }
    emit audioTracks(tracks);
}
