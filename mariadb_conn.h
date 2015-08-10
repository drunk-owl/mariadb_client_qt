#ifndef MARIADB_CONN_H
#define MARIADB_CONN_H

#include <QObject>
#include <functional>
#include <QSocketNotifier>
#include <QTimer>

#include "mariadb_conn_params.h"
#include "mariadb_result.h"

class MariaDbConn : public QObject
{
    Q_OBJECT
private:
    enum WaitKinds
    {
        WaitConnect,
        WaitQuery,
        WaitResult
    };

public:
    MariaDbConn(QObject *parent);
    ~MariaDbConn();

    void beginConnect(const MariaDbConnParams &conn_params);
    void beginQuery(const QByteArray &query);

    std::function<void()> onConnected;
    std::function<void(const QString&)> onConnectingError;
    std::function<void(const MariaDbResult&)> onResult;

private:
    void continueConnect(int status);
    void processConnect(int new_status, MYSQL *ret);

    void continueQuery(int status);
    void processQuery(int new_status, int err);

    void beginStoreResult();
    void continueStoreResult(int status);
    void processStoreResult(int new_status, MYSQL_RES *res);

    void processReady(int status);

    void setWaiters(int new_status, WaitKinds wait_kind);

    MYSQL *conn;
    int cur_sock;
    QSocketNotifier *rn, *wn, *en;
    QTimer tmr;
    WaitKinds wait_kind;

    QByteArray cur_query;

private slots:
    void onRead();
    void onWrite();
    void onExcept();
    void onTimeout();

};

#endif // MARIADB_CONN_H
