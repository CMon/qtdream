#include "qtdream.h"

#include <qtdream-gui/configurationdialog.h>
#include <qtdream-gui/log.h>

#include "ui_qtdream.h"

namespace {

int bRefRole = Qt::UserRole + 1;

bool lessEnigma2Event(const DreamboxApi::Enigma2Event & lhs, const DreamboxApi::Enigma2Event & rhs)
{
    return lhs.start < rhs.start;
}

}

QtDream::QtDream(QWidget *parent) :
    QMainWindow(parent),
    dBox_("192.168.111.27", 80, 8001, this),
    audioOutput_(Phonon::VideoCategory),
    ui(new Ui::QDream)
{
    ui->setupUi(this);
    ui->vsLocalVolume->setAudioOutput(&audioOutput_);

    connect(ui->action_Quit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->pbRefreshBouquets, SIGNAL(clicked()), &dBox_, SLOT(requestBouquets()));
    connect(ui->twBouquets, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(senderSelected(QTreeWidgetItem*)));

    connect(&dBox_, SIGNAL(newBouquets(QList<DreamboxApi::Enigma2Service>)), this, SLOT(updateBouquets(QList<DreamboxApi::Enigma2Service>)));
    connect(&dBox_, SIGNAL(newSenders(QString,QList<DreamboxApi::Enigma2Service>)), this, SLOT(updateSenders(QString,QList<DreamboxApi::Enigma2Service>)));
    connect(&dBox_, SIGNAL(updateEPGs(QString,QList<DreamboxApi::Enigma2Event>)), this, SLOT(updateEPGs(QString,QList<DreamboxApi::Enigma2Event>)));
    connect(&dBox_, SIGNAL(streamingUrl(QString)), this, SLOT(onDBoxStreamingUrl(QString)));

    audioOutputPath_ = Phonon::createPath(&mediaObject_, &audioOutput_);
    Phonon::createPath(&mediaObject_, ui->vwDreambox);

    DreamboxManagementWidget * dmw = new DreamboxManagementWidget(this);
    ui->tabWidget->addTab(dmw, "Dreambox Management");
}

QtDream::~QtDream()
{
// used for sharedPointer
}

void QtDream::updateBouquets(const QList<DreamboxApi::Enigma2Service> & bouquets)
{
    foreach (const DreamboxApi::Enigma2Service & bouquet, bouquets) {
        if (services_.contains(bouquet.reference)) {
            // update
            ServiceData & service = services_[bouquet.reference];
            if (service.serviceName != bouquet.name)
            {
                service.serviceName = bouquet.name;
                service.item->setText(0, bouquet.name);
            }
        } else {
            // new
            ServiceData & service = services_[bouquet.reference];
            service.serviceName = bouquet.name;
            service.item =  new QTreeWidgetItem(QStringList() << bouquet.name);

            ui->twBouquets->addTopLevelItem(service.item);
        }
        dBox_.requestSenders(bouquet.reference);
    }
}

void QtDream::updateSenders(const QString & bouquet, const QList<DreamboxApi::Enigma2Service> & senders)
{
    foreach (const DreamboxApi::Enigma2Service & sender, senders) {

        if (services_.contains(sender.reference)) {
            // update
            ServiceData & service = services_[sender.reference];
            if (service.serviceName != sender.name)
            {
                service.serviceName = sender.name;
                service.item->setText(0, sender.name);
            }
        } else {
            // new
            ServiceData & service = services_[sender.reference];
            service.serviceName = sender.name;
            service.item =  new QTreeWidgetItem(QStringList() << sender.name);
            service.item ->setData(0, bRefRole, QVariant::fromValue(sender.reference));

            services_[bouquet].item->addChild(service.item);
        }
        dBox_.requestSenderDetails(sender.reference);
    }
}

void QtDream::updateSenderDetails(const QString & reference)
{
    ui->lblCurrentSender->setText(services_[reference].serviceName);

    const QList<DreamboxApi::Enigma2Event> & events = services_[reference].events;
    if (events.size() < 2) return;

    const QString percent = QString::number((double) events.at(0).start.secsTo(events.at(0).currentTime) / (double) events.at(0).duration * 100, 'f', 0);
    const QString epgTable =
            "<table>"
            "<tr><td>" + events.at(0).start.time().toString("hh:mm") + "</td><td>" + events.at(0).title + "</td><td>(" + percent + "%)</td></tr>"
            "<tr><td>" + events.at(1).start.time().toString("hh:mm") + "</td><td>" + events.at(1).title + "</td></tr>"
            "</table>";
    ui->lblEPGs->setText(epgTable);
}

void QtDream::updateEPGs(const QString & reference, const QList<DreamboxApi::Enigma2Event> & epgs)
{
    QList<DreamboxApi::Enigma2Event> epgCopy = epgs;
    qSort(epgCopy.begin(), epgCopy.end(), lessEnigma2Event);
    services_[reference].events = epgCopy;

    // check if current selected item has to change
    const QTreeWidgetItem * current = ui->twBouquets->currentItem();
    if (!current) return;

    const QString currentRef = current->data(0, bRefRole).toString();
    if (reference == currentRef) {
        updateSenderDetails(reference);
    }
}

void QtDream::on_pbWatch_clicked()
{
    dBox_.requestStreamingUrl(ui->twBouquets->currentItem()->data(0, bRefRole).toString());
}

void QtDream::onDBoxStreamingUrl(const QString & url)
{
    setWindowTitle(url);

    mediaObject_.stop();
    mediaObject_.setCurrentSource(Phonon::MediaSource(QUrl::fromEncoded(url.toUtf8())));
    mediaObject_.play();
}

void QtDream::senderSelected(QTreeWidgetItem * item)
{
    if (!item) return ;

    updateSenderDetails(item->data(0, bRefRole).toString());
}

void QtDream::on_aConfiguration_triggered()
{
    ConfigurationDialog cDialog(this);

    cDialog.show();
}

void QtDream::on_pbSwitchDevice_clicked()
{
}
