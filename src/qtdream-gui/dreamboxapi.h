#pragma once

#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPair>

class DreamboxApi : public QObject
{
    Q_OBJECT
public:
    explicit DreamboxApi(const QString & host, int port = 80, int streamPort = 8001, QObject *parent = 0);
    ~DreamboxApi ();

    class Enigma2Service
    {
    public:
        Enigma2Service() {}
        Enigma2Service(const QString & n, const QString & ref) : name(n), reference(ref){}

        QString name;
        QString reference;
    };

    class Enigma2Event
    {
    public:
        int       id;
        QDateTime start;
        int       duration;
        QDateTime currentTime;
        QString   title;
        QString   description;
        QString   extendedDescription;
        QString   reference;
        QString   serviceName;
    };

    class TunerInfo
    {
    public:
        QString name;
        QString type;
    };

    class HddInfo
    {
    public:
        QString model;
        QString capacity;
        QString free;
    };

    class GeneralBoxInfo
    {
    public:
        QString enigmaVersion;

        bool isDHCPenabled;
        QString ip;
        QString netmask;
        QString gateway;

        QString fpVersion;
        QString webInterfaceVersion;
        QList<TunerInfo> tunerInformations;

        QString hddModel;
        QString hddCapacity;
        QString hddFree;

        QString serviceName;
        QString serviceProvider;
        QString aspect;
        QString videoSize;
        QString serviceNamespace;

        int vpid;
        int apid;
        int pcrid;
        int pmtpid;
        int txtpid;
        int tsid;
        int onid;
        int sid;
    };

    class AudioTrack
    {
    public:
        QString description;
        int id;
        int pId;
        bool active;
    };

public slots:
    void requestGeneralBoxInfo() const;
    void requestCurrentAudioTracks() const;
    void requestCurrentChannel() const;
    void requestStreamingUrl(const QString & serviceReference) const;
    void requestVolume() const;
    void setVolume(int volume) const;
    void incVolume() const;
    void decVolume() const;
    void toggleMute() const;
    void requestBouquets() const;
    void requestSenders(const QString & bouquetReference) const ;
    void requestSenderDetails(const QString & reference) const;

signals:
    void generalBoxInfos(const QList<DreamboxApi::GeneralBoxInfo> & infos);
    void audioTracks(const QList<DreamboxApi::AudioTrack> & tracks);
    void streamingUrl(const QString & url);
    void currentChannel(const QString & reference);
    void volume(int vol);
    void muted(bool muted);
    void newBouquets(const QList<DreamboxApi::Enigma2Service> & bouquets);
    void newSenders(const QString & bouquet, const QList<DreamboxApi::Enigma2Service> & senders);
    void updateEPGs(const QString & sReference, const QList<DreamboxApi::Enigma2Event> & epgs);
    void errorString(const QString & errorMsg);

private slots:
    void error(QNetworkReply::NetworkError err);
    void generalBoxInfo();
    void volumeChanged();
    void currentChannel();
    void currentStreamingUrl();
    void currentAudioTracks();
    void newService();
    void updateSenderDetail();

private:
    void changeVolume(const QString & command) const;

private:
    QNetworkAccessManager *manager;
    QString requestBase_;
    QString streamBase_;
};
