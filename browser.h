#ifndef BROWSER_H
#define BROWSER_H

#include <QWidget>
#include <QSqlTableModel>
#include "ui_browserwidget.h"

class ConnectionWidget;
QT_FORWARD_DECLARE_CLASS(QTableView)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QTextEdit)
QT_FORWARD_DECLARE_CLASS(QSqlError)

class Browser: public QWidget, private Ui::Browser
{
    Q_OBJECT
public:
    Browser(QWidget *parent = 0);
    virtual ~Browser();

    QSqlError addConnection(const QString &driver, const QString &dbName, const QString &host,
                  const QString &user, const QString &passwd, int port = -1);

    QSqlDatabase currentDatabase() const;

    void insertRow();

    void updateActions();
    //void exec(QString &query);

public slots:
    void exec();
    void showTable(const QString &table);
    void showMetaData(const QString &table);
    void addConnection();
    void currentChanged() { updateActions(); }

    void Back();
    void Refresh();
    void on_insertRowAction_triggered()
    { insertRow(); }
    void on_ActionBack_triggered()
    { Back(); }
    void on_ActionRefresh_triggered()
    { Refresh(); }
    /*void on_connectionWidget_tableActivated(const QString &table)
    { showTable(table); }*/
    /*void on_connectionWidget_metaDataRequested(const QString &table)
    { showMetaData(table); }*/

signals:
    void statusMessage(const QString &message);

private:
    QString DBase;
};

class CustomModel: public QSqlTableModel
{
    Q_OBJECT
public:
    explicit CustomModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase()):QSqlTableModel(parent, db) {}
    QVariant data(const QModelIndex &idx, int role) const Q_DECL_OVERRIDE
    {
        if (role == Qt::BackgroundRole && isDirty(idx))
            return QBrush(QColor(Qt::yellow));
        return QSqlTableModel::data(idx, role);
    }

};

#endif
