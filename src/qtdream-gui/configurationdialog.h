#pragma once

#include <QDialog>

namespace Ui {
    class ConfigurationDialog;
}

class QTreeWidgetItem;

class ConfigurationDialog : public QDialog
{
    Q_OBJECT
public:
    ConfigurationDialog(QWidget * parent, const QString & xmlFile = "configuration.xml");
    ~ConfigurationDialog();

    QString getBoxesFile() { return boxesFile_; }
    QStringList getGroupNames() { return groupNames_; }

public slots:
    void accept();

private slots:
    void on_pbAddGroupname_clicked();
    void on_pbBrowseFiles_clicked();
    void on_pbRemoveSelected_clicked();

private:
    void loadConfiguration();
    void storeConfiguration();

private:
    QString xmlFile_;
    QString boxesFile_;
    QStringList groupNames_;

private:
    QSharedPointer<Ui::ConfigurationDialog> ui;
};
