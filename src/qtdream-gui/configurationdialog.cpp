#include "configurationdialog.h"

#include <qtdream-gui/log.h>

#include <QAbstractMessageHandler>
#include <QFile>
#include <QFileDialog>
#include <QtXml>
#include <QXmlItem>
#include <QXmlQuery>
#include <QXmlResultItems>

#include "ui_configuration.h"

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

ConfigurationDialog::ConfigurationDialog(QWidget *parent, const QString & xmlFile) :
    QDialog(parent),
    xmlFile_(xmlFile),
    ui(new Ui::ConfigurationDialog)
{
    ui->setupUi(this);

    loadConfiguration();
}

ConfigurationDialog::~ConfigurationDialog()
{
// used for sharedPointer
}

void ConfigurationDialog::storeConfiguration()
{
    QFile file(xmlFile_);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR("could not open file: %1", xmlFile_);
        return;
    }

    QXmlStreamWriter* xmlWriter = new QXmlStreamWriter();
    xmlWriter->setDevice(&file);
    xmlWriter->writeStartDocument();
    xmlWriter->setAutoFormatting(true);
    xmlWriter->setAutoFormattingIndent(1);

    xmlWriter->writeStartElement("configuration");
    xmlWriter->writeTextElement("boxesFile", boxesFile_);

    xmlWriter->writeStartElement("groups");
    foreach (const QString & group, groupNames_) {
        xmlWriter->writeTextElement("name", group);
    }
    xmlWriter->writeEndElement(); // groups

    xmlWriter->writeEndElement(); // configuration
    xmlWriter->writeEndDocument();
    delete xmlWriter;

    file.close();
}

void ConfigurationDialog::loadConfiguration()
{
    QFile file(xmlFile_);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_WARN("could not open configuration file starting new one...");
        return;
    }

    QXmlQuery query;
    query.setFocus(&file);
    query.setQuery( "/configuration/groups" );
    query.setMessageHandler(new DebuggingMessageHandler(this));

    groupNames_.clear();
    QXmlResultItems result;
    query.evaluateTo(&result);
    QXmlItem item(result.next());
    while (!item.isNull()) {
        query.setFocus(item);
        QString groupName = evaluateItem(query, "name", false);
        if (!groupName.isEmpty()) {
            groupNames_ << groupName;
        }
        item = result.next();
    }
    boxesFile_ = evaluateItem(query, "/configuration/boxesFile", false);

    // set ui elements
    ui->leBoxesFile->setText(boxesFile_);
    foreach (const QString & group, groupNames_) {
        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setText(0, group);
        ui->twDreamboxes->addTopLevelItem(item);
    }

    file.close();
}

void ConfigurationDialog::accept()
{
    storeConfiguration();

    boxesFile_ = ui->leBoxesFile->text();
    groupNames_.clear();
    for(int i = 0; i < ui->twDreamboxes->topLevelItemCount(); ++i) {
        const QString name = ui->twDreamboxes->topLevelItem(i)->text(0);
        if (name.isEmpty()) continue;
        groupNames_ << name;
    }

    QDialog::accept();
}

void ConfigurationDialog::on_pbAddGroupname_clicked()
{
    if (ui->leGroupname->text().isEmpty()) return;

    QTreeWidgetItem * item = new QTreeWidgetItem();
    item->setText(0, ui->leGroupname->text());

    ui->twDreamboxes->addTopLevelItem(item);
}

void ConfigurationDialog::on_pbRemoveSelected_clicked()
{
    QTreeWidgetItem * item = ui->twDreamboxes->currentItem();
    if (!item) return;

    QTreeWidgetItem * item2 = ui->twDreamboxes->takeTopLevelItem(ui->twDreamboxes->indexOfTopLevelItem(item));
    delete item2;
}

void ConfigurationDialog::on_pbBrowseFiles_clicked()
{
    const QString boxesFile = QFileDialog::getSaveFileName(this, tr("Select file to store boxes in"), QString(), tr("XML (*.xml)"));
    if (boxesFile.isEmpty()) return;

    boxesFile_ = boxesFile;
    ui->leBoxesFile->setText(boxesFile_);
}
