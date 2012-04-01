#include "qtdream.h"

#include <qtdream-gui/configurationdialog.h>
#include <qtdream-gui/log.h>

#include "ui_qtdream.h"

#include <QTimer>

namespace {

int bRefRole = Qt::UserRole + 1;

bool lessEnigma2Event(const DreamboxApi::Enigma2Event & lhs, const DreamboxApi::Enigma2Event & rhs)
{
    return lhs.start < rhs.start;
}

}

QtDream::QtDream(QWidget *parent) :
    QMainWindow(parent),
    dBox_("192.168.1.34", 80, 8001, this),
    audioOutput_(Phonon::VideoCategory),
    ui(new Ui::QDream)
{
    ui->setupUi(this);
    ui->vsLocalVolume->setAudioOutput(&audioOutput_);

    connect(ui->action_Quit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->action_Configuration, SIGNAL(triggered()), this, SLOT(on_aConfiguration_triggered()));
    connect(ui->pbRefreshBouquets, SIGNAL(clicked()), &dBox_, SLOT(requestBouquets()));
    connect(ui->twBouquets, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(senderSelected(QTreeWidgetItem*)));
    connect(ui->twBouquets, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(senderSelected(QTreeWidgetItem*)));
    connect(ui->twBouquets, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(senderSelected(QTreeWidgetItem*)));

    connect(&dBox_, SIGNAL(newBouquets(QList<DreamboxApi::Enigma2Service>)), this, SLOT(updateBouquets(QList<DreamboxApi::Enigma2Service>)));
    connect(&dBox_, SIGNAL(newSenders(QString,QList<DreamboxApi::Enigma2Service>)), this, SLOT(updateSenders(QString,QList<DreamboxApi::Enigma2Service>)));
    connect(&dBox_, SIGNAL(updateEPGs(QString,QList<DreamboxApi::Enigma2Event>)), this, SLOT(updateEPGs(QString,QList<DreamboxApi::Enigma2Event>)));
    connect(&dBox_, SIGNAL(streamingUrl(QString)), this, SLOT(onDBoxStreamingUrl(QString)));

    audioOutputPath_ = Phonon::createPath(&mediaObject_, &audioOutput_);
    Phonon::createPath(&mediaObject_, ui->vwDreambox);
    connect(&mediaObject_, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChanged(Phonon::State,Phonon::State)));

    DreamboxManagementWidget * dmw = new DreamboxManagementWidget(this);
    ui->tabWidget->addTab(dmw, "Dreambox Management");

    QTimer * updateSenderTimer = new QTimer(this);
    updateSenderTimer->setInterval(30 * 1000);
    updateSenderTimer->setSingleShot(false);
    connect(updateSenderTimer, SIGNAL(timeout()), this, SLOT(checkForUpdatedServiceData()));
}

QtDream::~QtDream()
{
// used for sharedPointer
}

void QtDream::updateBouquets(const QList<DreamboxApi::Enigma2Service> & bouquets)
{
    LOG_DEBUG_FUNCTION;
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
    LOG_DEBUG_FUNCTION;
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

void QtDream::updateCurrentlyPlaying(const QString &reference)
{
    // TODO connect it to the actual signal from the dbox api and then be sure to call it at the right times (start, regular basis and change)
    if (!services_.contains(reference)) return;
    ui->lblCurrentSender->setText(services_[reference].serviceName + " " + services_[reference].events.first().title);
}

void QtDream::on_pbWatch_clicked()
{
    dBox_.requestStreamingUrl(ui->twBouquets->currentItem()->data(0, bRefRole).toString());
}

void QtDream::onDBoxStreamingUrl(const QString & url)
{
    setWindowTitle(url);

    LOG_DEBUG("Start playback of %1", url);
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

    cDialog.exec();
}

void QtDream::on_pbSwitchDevice_clicked()
{
    if (ui->leDevice->text().isEmpty()) return;
    // @TODO: sanity Check if ip or hostname
    dBox_.setConnectionInfos(ui->leDevice->text());
}

void QtDream::checkForUpdatedServiceData()
{
    LOG_DEBUG_FUNCTION;
    LOG_DEBUG("This method has to be implemented");
    /// @TODO
    // here we check if one of the services in services_ has old data
    // for example currently playing stuff which is already through
}

void QtDream::stateChanged(Phonon::State newState, Phonon::State /*oldState*/)
{
    switch(newState) {
        case Phonon::ErrorState:
            LOG_ERROR("An error occured: %1", mediaObject_.errorString());
            break;
        default:
            break;
    }
}
