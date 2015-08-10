#include "mariadb_conn_pool.h"
#include "mariadb_conn.h"
#include <QTimer>
#include <QDebug>

#define RECONN_TIMEOUT  100//ms

MariaDbConnPool::MariaDbConnPool(const MariaDbConnParams conn_params, unsigned max_conn_count, QObject *parent) :
    QObject(parent),
    conn_params(conn_params), max_conn_count(max_conn_count),
    next_query_id(1)
{
}

quint64 MariaDbConnPool::pushQuery(const QByteArray &query, MariaDbResultCallback cb_func)
{
    quint64 query_id=next_query_id++;
    query_queue+=QPair<QByteArray, ResultMethod>(query, ResultMethod(query_id, cb_func));

    if(!idle_conns.isEmpty())
    {
        beginNextQuery();
    }
    else if((unsigned)(new_conns.count()+busy_conns.count()) < max_conn_count)
    {
        createNewConn();
    }

    return query_id;
}

quint64 MariaDbConnPool::pushQuery(const QString &query, MariaDbResultCallback cb_func)
{
    return pushQuery(query.toUtf8(), cb_func);
}

void MariaDbConnPool::createNewConn()
{
    MariaDbConn *conn=new MariaDbConn(this);
    conn->onConnected=[this, conn]() {
        onConnected(conn);
    };
    conn->onConnectingError=[this, conn](const QString &error) {
        onConnectingError(conn, error);
    };
    conn->onResult=[this, conn](const MariaDbResult &result) {
        onResult(conn, result);
    };
    new_conns+=conn;
    conn->beginConnect(conn_params);
}

void MariaDbConnPool::beginNextQuery()
{
    MariaDbConn *conn=idle_conns.takeLast();
    QPair<QByteArray, ResultMethod> query=query_queue.takeFirst();
    busy_conns.insert(conn, query.second);
    conn->beginQuery(query.first);
}

void MariaDbConnPool::finishError()
{
    if(!query_queue.isEmpty())
    {
        if(!idle_conns.isEmpty())
        {
            beginNextQuery();
        }
        else if((unsigned)(new_conns.count()+busy_conns.count()) < max_conn_count)
        {
            QTimer::singleShot(RECONN_TIMEOUT, this, SLOT(createNewConn()));
        }
    }
}

void MariaDbConnPool::onConnected(MariaDbConn *conn)
{
    new_conns.removeOne(conn);
    idle_conns+=conn;
    if(!query_queue.isEmpty())
    {
        beginNextQuery();
    }
}

void MariaDbConnPool::onConnectingError(MariaDbConn *conn, const QString &error)
{
    new_conns.removeOne(conn);
    idle_conns.removeOne(conn);
    conn->deleteLater();
    emit this->error(error);
    finishError();
}

void MariaDbConnPool::onResult(MariaDbConn *conn, const MariaDbResult &result)
{
    auto bc_it=busy_conns.find(conn);
    Q_ASSERT(bc_it!=busy_conns.end());

    if(bc_it->cb_func==0)
    {
        emit this->result(result, bc_it->query_id);
    }
    else
    {
        bc_it->cb_func(result, bc_it->query_id);
    }
    busy_conns.erase(bc_it);

    idle_conns+=conn;
    if(!query_queue.isEmpty())
    {
        beginNextQuery();
    }
}
