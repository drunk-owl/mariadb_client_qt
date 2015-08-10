#ifndef MARIADB_CONN_BLOCK_H
#define MARIADB_CONN_BLOCK_H

#include <mysql/mysql.h>
#include "mariadb_conn_params.h"
#include "mariadb_result.h"

class MariaDbConnBlock
{
public:
    MariaDbConnBlock();
    ~MariaDbConnBlock();

    bool connect(const MariaDbConnParams &conn_params);
    MariaDbResult exec(const QByteArray &query);
    MariaDbResult exec(const QString &query);

    QString errorString() const;

private:
    MYSQL *conn;

};

#endif // MARIADB_CONN_BLOCK_H
