#include "dreamboxmanagementwidget.h"

#include <qtdream-gui/log.h>

#include <QAbstractMessageHandler>
#include <QCloseEvent>
#include <QFile>
#include <QtXml>
#include <QXmlItem>
#include <QXmlQuery>
#include <QXmlResultItems>
#include <QXmlStreamWriter>

#include "ui_dreamboxmanagementwidget.h"

namespace {

class DebuggingMessageHandler : public QAbstractMessageHandler
{
public:
    DebuggingMessageHandler(QObject * parent = 0) : QAbstractMessageHandler(parent) {}
    void handleMessage(QtMsgType type, const QString & description, const QUrl & identifier = QUrl(), const QSourceLocation & sourceLocation = QSourceLocation())
    {
        LOG_DEBUG("Type: %1 Description: %2 Identifier: %3 LocationLine: %4 LocationCol: %5",
                  type, description, identifier.toString(), sourceLocation.line(), sourceLocation.column());
    }
};

QString evaluateItem(QXmlQuery & query, const QString & queryString, bool requestString = true)
{
    QString data;
    if (requestString) query.setQuery(queryString + "/string()");
    else               query.setQuery(queryString);
    query.evaluateTo(&data);
    return data.trimmed();
}

}

DreamboxManagementWidget::DreamboxManagementWidget(QWidget * parent, const QString & boxesFile) :
    QWidget(parent),
    boxFile_(boxesFile),
    mgmServer_(this),
    ui(new Ui::DreamboxManagementWidget)
{
    ui->setupUi(this);

    connect(&mgmServer_, SIGNAL(newBox(QString)), this, SLOT(newConnectedBox(QString)));
}

DreamboxManagementWidget::~DreamboxManagementWidget()
{
    // used for sharedPointer
}

void DreamboxManagementWidget::newConnectedBox(const QString &id)
{
    // FIXME: figure out which group this box belongs too and ad it there, for now we only add the box
    QTreeWidgetItem * item = new QTreeWidgetItem();

    // FIXME: get name for id
    item->setText(0, "");
    item->setText(1, id);

    ui->twBoxes->addTopLevelItem(item);
}

void DreamboxManagementWidget::closeEvent (QCloseEvent * event)
{
    QFile file(boxFile_);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR("could not open file: %1", boxFile_);
        event->accept();
        return;
    }

    QXmlStreamWriter* xmlWriter = new QXmlStreamWriter();
    xmlWriter->setDevice(&file);
    xmlWriter->writeStartDocument();
    xmlWriter->setAutoFormatting(true);
    xmlWriter->setAutoFormattingIndent(1);

    xmlWriter->writeStartElement("boxes");
    foreach (const QString & id, boxes_.keys()) {
        const BoxInfo & info = boxes_[id];
        xmlWriter->writeStartElement("box");
        xmlWriter->writeTextElement("id",            id);
        xmlWriter->writeTextElement("name",          info.name);
        xmlWriter->writeTextElement("group",         info.groupName);
        xmlWriter->writeTextElement("notes",         info.notes);
        xmlWriter->writeTextElement("sshTunnelPort", QString::number(info.sshTunnelPort));
        xmlWriter->writeEndElement(); // box
    }
    xmlWriter->writeEndElement(); // boxes

    xmlWriter->writeEndDocument();
    delete xmlWriter;

    file.close();
    event->accept();
}

void DreamboxManagementWidget::showEvent (QShowEvent * event)
{
    QFile file(boxFile_);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_WARN("could not open configuration file starting new one...");
        event->accept();
        return;
    }

    QXmlQuery query;
    query.setFocus(&file);
#ifdef DEBUG
    query.setMessageHandler(new DebuggingMessageHandler(this));
#endif

    boxes_.clear();
    query.setQuery("/boxes/box");
    QXmlResultItems result;
    query.evaluateTo(&result);
    QXmlItem item(result.next());
    while (!item.isNull()) {
        BoxInfo info;
        query.setFocus(item);

        const QString boxId = evaluateItem(query, "id");
        info.name           = evaluateItem(query, "name");
        info.groupName      = evaluateItem(query, "group");
        info.notes          = evaluateItem(query, "notes");
        info.sshTunnelPort  = evaluateItem(query, "sshTunnelPort").toInt();
        boxes_[boxId] = info;

        item = result.next();
    }

    file.close();
    event->accept();
}

void DreamboxManagementWidget::on_pbSave_clicked()
{
    const QString boxId = ui->lblId->text();
    if (boxId.isEmpty()) return;

    boxes_[boxId].groupName     = ui->cbGroup->currentText();
    boxes_[boxId].name          = ui->leName->text();
    boxes_[boxId].notes         = ui->teNotes->toHtml();
    boxes_[boxId].sshTunnelPort = ui->sbSSHPort->value();

    if (boxes_[boxId].name != ui->twBoxes->currentItem()->text(Name)) {
        ui->twBoxes->currentItem()->setText(Name, boxes_[boxId].name);
    }
}

void DreamboxManagementWidget::on_pbCancel_clicked()
{
    const QString boxId = ui->lblId->text();
    if (boxId.isEmpty()) return;
    setSelectedBox(boxId);
}

void DreamboxManagementWidget::on_pbConnect_clicked()
{
    const QString boxId = ui->lblId->text();
    if (boxId.isEmpty()) return;
    const int port = ui->sbSSHPort->value();

    QString answer = mgmServer_.createSSHTunnel(boxId, port);
    LOG_DEBUG(answer);
}

void DreamboxManagementWidget::on_twBoxes_itemActivated(QTreeWidgetItem * item, int /*column*/)
{
    if (!item) return;

    const QString boxId = item->text(Id);
    if (boxId.isEmpty()) return;

    setSelectedBox(boxId);
}

void DreamboxManagementWidget::setSelectedBox(const QString & boxId)
{
    if (boxId.isEmpty()) return;

    ui->cbGroup->setCurrentIndex(ui->cbGroup->findText(boxes_[boxId].groupName));
    ui->leName->setText(boxes_[boxId].name);
    ui->teNotes->setHtml(boxes_[boxId].notes);
    ui->sbSSHPort->setValue(boxes_[boxId].sshTunnelPort);
}
