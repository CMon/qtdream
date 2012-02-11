#pragma once

#include <qtdream-gui/dreamboxapi.h>
#include <qtdream-gui/dreamboxmanagementwidget.h>

#include <Phonon/AudioOutput>
#include <Phonon/MediaObject>
#include <QtGui/QMainWindow>

class QTreeWidgetItem;

namespace Ui {
    class QDream;
}

struct ServiceData {
    QString             serviceName;
    QTreeWidgetItem *   item;
    QList<DreamboxApi::Enigma2Event> events;
};


class QtDream : public QMainWindow
{
    Q_OBJECT

public:
    explicit QtDream(QWidget *parent = 0);
    ~QtDream();

private:
    void connectToDreambox();
    void updateSenderDetails(const QString & reference);

private slots:
    void updateBouquets(const QList<DreamboxApi::Enigma2Service> & bouquets);
    void updateSenders(const QString & bouquet, const QList<DreamboxApi::Enigma2Service> & senders);
    void updateEPGs(const QString & reference, const QList<DreamboxApi::Enigma2Event> & epgs);

    void onDBoxStreamingUrl(const QString & url);

    void on_pbWatch_clicked();

    void senderSelected(QTreeWidgetItem * item);

    void on_aConfiguration_triggered();

    void on_pbSwitchDevice_clicked();

private:
    QHash<QString /*service reference*/, ServiceData>  services_;
    DreamboxApi dBox_;
    Phonon::MediaObject  mediaObject_;
    Phonon::AudioOutput  audioOutput_;
    Phonon::Path         audioOutputPath_;

private:
    QSharedPointer<Ui::QDream> ui;
};
