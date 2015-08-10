#ifndef MARIADB_CONN_POOL_H
#define MARIADB_CONN_POOL_H

#include <QObject>
#include <functional>

#include "mariadb_conn_params.h"
#include "mariadb_result.h"

class MariaDbConn;

typedef std::function<void(MariaDbResult,quint64)> MariaDbResultCallback;

class MariaDbConnPool : public QObject
{
    Q_OBJECT
private:
    class ResultMethod;

public:
    MariaDbConnPool(const MariaDbConnParams conn_params, unsigned max_conn_count, QObject *parent);

    quint64 pushQuery(const QByteArray &query, MariaDbResultCallback cb_func = MariaDbResultCallback());
    quint64 pushQuery(const QString &query, MariaDbResultCallback cb_func = MariaDbResultCallback());

private:
    void beginNextQuery();
    void finishError();

    void onConnected(MariaDbConn *conn);
    void onConnectingError(MariaDbConn *conn, const QString &error);

    void onResult(MariaDbConn *conn, const MariaDbResult &result);

    const MariaDbConnParams conn_params;
    const unsigned max_conn_count;

    QList<MariaDbConn*> new_conns, idle_conns;
    QMap<MariaDbConn*, ResultMethod> busy_conns;

    quint64 next_query_id;
    QList<QPair<QByteArray, ResultMethod> > query_queue;

signals:
    void result(const MariaDbResult &result, quint64 query_id);
    void error(const QString &error);

public slots:
    void createNewConn();

};

class MariaDbConnPool::ResultMethod
{
public:
    ResultMethod(quint64 query_id, MariaDbResultCallback cb_func) :
        query_id(query_id), cb_func(cb_func) {}

    quint64 query_id;
    MariaDbResultCallback cb_func;
};

#endif // MARIADB_CONN_POOL_H
