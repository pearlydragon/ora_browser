#include "browser.h"
#include <QtCore>
#include <QtWidgets>
#include <QtSql>
#include <QFile>
#include <QDebug>

//Версия 1,0. Умеет различать базы, поданные на вход и выполнять к ним коннект.



void addConnectionsFromCommandline(const QStringList &args, Browser *browser)
{
    for (int i = 1; i < args.count(); ++i) {
        QUrl url(args.at(i), QUrl::TolerantMode);
        if (!url.isValid()) {
            qWarning("Invalid URL: %s", qPrintable(args.at(i)));
            continue;
        }
        QSqlError err = browser->addConnection(url.scheme(), url.path().mid(1), url.host(),
                                               url.userName(), url.password(), url.port(-1));
        if (err.type() != QSqlError::NoError)
            qDebug() << "Unable to open connection:" << err;
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString con_db = "";
    QString con_attr = "";
    QString con_host = "";
    QString con_pass = "";
    int con_port = 1521;
    //--------------------------------------------files for connection resolve
    //QString constring = "КЯ-44 (Железнодорожников 14)";
    QString constring = argv[1];
    qDebug() << argv[1];
    QString homepwd = QDir::currentPath();
    QDir::setCurrent("/windows/u/Domino8/PROJECT.QT/ETK/DataBase");
    QFile listdb("./Contence.QT");
    if (!listdb.open(QIODevice::ReadOnly | QIODevice::Text))
            qDebug() << "File listdb not exist";
    QDir::setCurrent(homepwd);
    QTextStream in(&listdb);
    QString line = in.readLine();
    QStringList result;
    while (!line.isNull()) {
        if ( line.indexOf(constring, Qt::CaseInsensitive) != -1){
            result.append(constring);
            line = in.readLine();
            int pos = line.indexOf("v=")+2;
            con_attr = line.mid(pos);
            result.append(line);
            line = in.readLine();
            line = in.readLine();
            pos = line.indexOf("v=")+2;
            con_host = line.mid(pos);
            result.append(line);
            line = in.readLine();
            pos = line.indexOf("v=")+2;
            con_pass = line.mid(pos);
            result.append(line);
            break;
        }
        line = in.readLine();
    }

    QDir::setCurrent("/windows/u/Domino8/BIN");
    QFile tns("./tnsnames.ora");
    if (!tns.open(QIODevice::ReadOnly | QIODevice::Text))
            qDebug() << "File tns not exist";
    QTextStream in_tns(&tns);
    line = in_tns.readLine();
    result.clear();
    QString con_attr_temp = con_attr+" =";
    while (!line.isNull()) {
        if ( line.indexOf(con_attr_temp) != -1){
            line = in_tns.readLine();
            line = in_tns.readLine();
            line = in_tns.readLine();
            QRegExp rx("\\d+(.)\\d+(.)\\d+(.)\\d+");
            if (rx.indexIn(line) != -1){
            //qDebug() << "1:" << rx.indexIn(line);
            con_host = rx.cap(0);
            line = in_tns.readLine();
            line = in_tns.readLine();
            line = in_tns.readLine();
            QRegExp rx1("(= )\\S+");
            rx1.indexIn(line);
            con_db = rx1.cap(0);
            con_db = con_db.remove(QRegExp("(= )"));
            con_db = con_db.left(con_db.length()-1);
            if (line.indexOf("DB1.ETK.LOCAL") != -1)
            {
                con_db = "DB1";
            }
            break;
            }
        }
        line = in_tns.readLine();
    }
    QDir::setCurrent(homepwd);
    //--------------------------------------------files for connection resolve

    QMainWindow mainWin;
    mainWin.setWindowTitle(QObject::tr("Anythin PLAN"));

    Browser browser(&mainWin);
    mainWin.setCentralWidget(&browser);

    QObject::connect(&browser, SIGNAL(statusMessage(QString)),
                     mainWin.statusBar(), SLOT(showMessage(QString)));
    //addConnectionsFromCommandline(app.arguments(), &browser);
    //browser.addConnection("QOCI", "172.18.0.22", "ETK00", "ETK00", "ETK00", 1521);

    QSqlError err0 = browser.addConnection("QOCI", con_db, con_host, con_pass, con_pass, con_port);
    if (err0.type() != QSqlError::NoError){
        qDebug() << "Unable to open connection:" << err0;
        browser.statusMessage("Connect to ETK00 failed");
    }else
        browser.statusMessage("Connect to ETK00 sucessful");
    browser.exec();
    mainWin.show();
    if (QSqlDatabase::connectionNames().isEmpty())
        QMetaObject::invokeMethod(&browser, "addConnection", Qt::QueuedConnection);

    return app.exec();
}
