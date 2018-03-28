#include "browser.h"
#include <QtWidgets>
#include <QtSql>

Browser::Browser(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    if (QSqlDatabase::drivers().isEmpty())
        QMessageBox::information(this, tr("No database drivers found"), tr(""));

    emit statusMessage(tr("Ready."));
    connect(backButton,SIGNAL(clicked()),this,SLOT(Back()));
    connect(refreshButton,SIGNAL(clicked()),this,SLOT(Refresh()));
}

Browser::~Browser()
{
}

void Browser::Back()
{
    close();
    qApp->exit();
}

void Browser::Refresh()
{
    Browser::exec();
}

void Browser::exec()
{
    QSqlQueryModel *model = new QSqlQueryModel(table);
    model->setQuery(QSqlQuery("select (select DOMINO.DOMINOUIDTOSTRING(d.ID) from dual), \
                              d.NAME, \
                              (select FIX_DATE from MAIL_EXP_INFO where NODE = d.ID), \
                              (select LAST_DATE from MAIL_EXP_INFO where NODE = d.ID), \
                              (select CONFIRMATION from MAIL_CONFIRMATION where NODE = d.ID), \
                              (select LAST_DATE from MAIL_IMP_INFO where NODE = d.ID), \
                              (select LAST_DATE from MAIL_IMP_FILE_INFO where NODE = d.ID) \
                              from DB1_DOCUMENT d \
                              where \
                              d.TYPE = 1507357 \
                              and d.CLASS = 1507329 \
                              and d.DOC_DATE >= to_date('01.01.2007','DD.MM.YYYY') \
                              and d.F1507355 = HEXTORAW('07D2000307D20002') \
                              ORDER BY 2 ASC", QSqlDatabase::database(DBase)));
    table->setModel(model);
    //int colCount=model->columnCount();
    model->setHeaderData(0, Qt::Horizontal, "UID");
    model->setHeaderData(1, Qt::Horizontal, "Наименование");
    model->setHeaderData(2, Qt::Horizontal, "Дата фиксации");
    model->setHeaderData(3, Qt::Horizontal, "Дата отправки");
    model->setHeaderData(4, Qt::Horizontal, "Дата подтверждения");
    model->setHeaderData(5, Qt::Horizontal, "Дата прихода");
    model->setHeaderData(6, Qt::Horizontal, "Дата последнего \nобработанного пакета");
        table->resizeColumnsToContents();//---------------------------------------------------------------------------

    if (model->lastError().type() != QSqlError::NoError)
        emit statusMessage(model->lastError().text());
    else if (model->query().isSelect())
        emit statusMessage(tr("Query OK."));
    else
        emit statusMessage(tr("Query OK, number of affected rows: %1").arg(
                           model->query().numRowsAffected()));

    updateActions();

}

QSqlError Browser::addConnection(const QString &driver, const QString &dbName, const QString &host,
                            const QString &user, const QString &passwd, int port)
{
    static int cCount = 0;

    QSqlError err;
    QSqlDatabase db = QSqlDatabase::addDatabase(driver, QString("Browser%1").arg(++cCount));
    db.setDatabaseName(dbName);
    db.setHostName(host);
    db.setPort(port);
    db.setUserName(user);
    db.setPassword(passwd);
    if (!db.open()) {
        err = db.lastError();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(QString("Browser%1").arg(cCount));


    }
    DBase = db.connectionName();
    return err;
}

void Browser::addConnection()
{

}

void Browser::showTable(const QString &t)
{
    QSqlTableModel *model = new CustomModel(table, Browser::currentDatabase());
    model->setEditStrategy(QSqlTableModel::OnRowChange);
    model->setTable(Browser::currentDatabase().driver()->escapeIdentifier(t, QSqlDriver::TableName));
    //model->setTable(QSqlDatabase::database(DBase).driver()->escapeIdentifier(t, QSqlDriver::TableName));
    model->select();
    if (model->lastError().type() != QSqlError::NoError)
        emit statusMessage(model->lastError().text());
    table->setModel(model);
    table->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);

    connect(table->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentChanged()));
    updateActions();
}

void Browser::showMetaData(const QString &t)
{
    QSqlRecord rec = Browser::currentDatabase().record(t);
    QStandardItemModel *model = new QStandardItemModel(table);

    model->insertRows(0, rec.count());
    model->insertColumns(0, 7);

    model->setHeaderData(0, Qt::Horizontal, "Fieldname");
    model->setHeaderData(1, Qt::Horizontal, "Type");
    model->setHeaderData(2, Qt::Horizontal, "Length");
    model->setHeaderData(3, Qt::Horizontal, "Precision");
    model->setHeaderData(4, Qt::Horizontal, "Required");
    model->setHeaderData(5, Qt::Horizontal, "AutoValue");
    model->setHeaderData(6, Qt::Horizontal, "DefaultValue");


    for (int i = 0; i < rec.count(); ++i) {
        QSqlField fld = rec.field(i);
        model->setData(model->index(i, 0), fld.name());
        model->setData(model->index(i, 1), fld.typeID() == -1
                ? QString(QMetaType::typeName(fld.type()))
                : QString("%1 (%2)").arg(QMetaType::typeName(fld.type())).arg(fld.typeID()));
        model->setData(model->index(i, 2), fld.length());
        model->setData(model->index(i, 3), fld.precision());
        model->setData(model->index(i, 4), fld.requiredStatus() == -1 ? QVariant("?")
                : QVariant(bool(fld.requiredStatus())));
        model->setData(model->index(i, 5), fld.isAutoValue());
        model->setData(model->index(i, 6), fld.defaultValue());
    }

    table->setModel(model);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    updateActions();
}

void Browser::insertRow()
{
    QSqlTableModel *model = qobject_cast<QSqlTableModel *>(table->model());
    if (!model)
        return;

    QModelIndex insertIndex = table->currentIndex();
    int row = insertIndex.row() == -1 ? 0 : insertIndex.row();
    model->insertRow(row);
    insertIndex = model->index(row, 0);
    table->setCurrentIndex(insertIndex);
    table->edit(insertIndex);
}

void Browser::updateActions()
{
    QSqlTableModel * tm = qobject_cast<QSqlTableModel *>(table->model());
    bool enableIns = tm;
    bool enableDel = enableIns && table->currentIndex().isValid();

    insertRowAction->setEnabled(enableIns);
    deleteRowAction->setEnabled(enableDel);

    fieldStrategyAction->setEnabled(tm);
    rowStrategyAction->setEnabled(tm);
    manualStrategyAction->setEnabled(tm);
    submitAction->setEnabled(tm);
    revertAction->setEnabled(tm);
    selectAction->setEnabled(tm);

    if (tm) {
        QSqlTableModel::EditStrategy es = tm->editStrategy();
        fieldStrategyAction->setChecked(es == QSqlTableModel::OnFieldChange);
        rowStrategyAction->setChecked(es == QSqlTableModel::OnRowChange);
        manualStrategyAction->setChecked(es == QSqlTableModel::OnManualSubmit);
    }
}

QSqlDatabase Browser::currentDatabase() const
{
    return QSqlDatabase::database(DBase);
}
