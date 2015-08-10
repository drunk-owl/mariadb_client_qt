#ifndef MARIADB_CONN_PARAMS_H
#define MARIADB_CONN_PARAMS_H

#include <QString>

class MariaDbConnParams
{
public:
    MariaDbConnParams(const QString &host, unsigned port,
                      const QString &user, const QString &password,
                      const QString &db_name,
                      unsigned conn_timeout) :
        host(host.toUtf8()), user(user.toUtf8()), password(password.toUtf8()), db_name(db_name.toUtf8()),
        port(port), conn_timeout(conn_timeout) {}
    MariaDbConnParams() :
        port(0), conn_timeout(0) {}
    MariaDbConnParams(const MariaDbConnParams &oth) :
        host(oth.host), user(oth.user), password(oth.password), db_name(oth.db_name),
        port(oth.port), conn_timeout(oth.conn_timeout) {}

    QByteArray host, user, password, db_name;
    unsigned port, conn_timeout;
};

#endif // MARIADB_CONN_PARAMS_H
