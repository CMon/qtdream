#pragma once

#include <qtdream-gui/managementserver.h>

#include <QWidget>

class QCloseEvent;
class QShowEvent;
class QTreeWidgetItem;

namespace Ui {
    class DreamboxManagementWidget;
}

class BoxInfo
{
public:
    QString name;
    QString groupName;
    int     sshTunnelPort;
    QString notes;
};

class DreamboxManagementWidget : public QWidget
{
    Q_OBJECT

public:
    enum {
        Name,
        Id
    };

    DreamboxManagementWidget(QWidget * parent = 0, const QString & boxesFile = "boxes.xml");
    ~DreamboxManagementWidget();

public slots:
    void newConnectedBox(const QString & id);

protected:
    void closeEvent (QCloseEvent * event);
    void showEvent (QShowEvent * event);

private slots:
    void on_pbSave_clicked();
    void on_pbCancel_clicked();
    void on_pbConnectSSH_clicked();
    void on_twBoxes_itemActivated(QTreeWidgetItem * item, int column);

private:
    void setSelectedBox(const QString & boxId);

private:
    QString                 boxFile_;
    ManagementServer        mgmServer_;
    QHash<QString, BoxInfo> boxes_;

private:
    QSharedPointer<Ui::DreamboxManagementWidget> ui;
};

