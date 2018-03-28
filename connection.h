#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlTableModel>

class sqlconnection
{
public:
    sqlconnection() {}
private:
    QString DB_NAME;
    QString DB_HOST;
    QString DB_USER;
    QString DB_PORT;
};

#endif // CONNECTION_H

