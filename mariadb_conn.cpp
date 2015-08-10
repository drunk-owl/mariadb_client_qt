#include "mariadb_conn.h"

MariaDbConn::MariaDbConn(QObject *parent) :
    QObject(parent),
    conn(0), cur_sock(-1), rn(0), wn(0), en(0)
{
}

MariaDbConn::~MariaDbConn()
{
    if(conn) mysql_close(conn);
}

void MariaDbConn::beginConnect(const MariaDbConnParams &conn_params)
{
    conn=mysql_init(0);
    Q_ASSERT_X(conn!=0, __FUNCTION__, "mysql_init failed");

    mysql_options(conn, MYSQL_OPT_NONBLOCK, 0);
    mysql_options(conn, MYSQL_OPT_RECONNECT, (const void*)"1");
    if(conn_params.conn_timeout>0) mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, (const void*)&conn_params.conn_timeout);
    mysql_options(conn, MYSQL_SET_CHARSET_NAME, (const void*)"utf8");

    MYSQL *ret;
    int new_status=mysql_real_connect_start(&ret, conn,
                                            conn_params.host.data(),
                                            conn_params.user.data(),
                                            conn_params.password.data(),
                                            conn_params.db_name.data(),
                                            conn_params.port,
                                            0, 0);

    tmr.setSingleShot(true);
    connect(&tmr, SIGNAL(timeout()), SLOT(onTimeout()));

    processConnect(new_status, ret);
}

void MariaDbConn::continueConnect(int status)
{
    MYSQL *ret;
    int new_status=mysql_real_connect_cont(&ret, conn, status);
    processConnect(new_status, ret);
}

void MariaDbConn::processConnect(int new_status, MYSQL *ret)
{
    if(new_status==0)
    {
        if(ret)
        {
            onConnected();
        }
        else
        {
            onConnectingError(QString::fromUtf8(mysql_error(conn)));
        }
    }
    else
    {
        setWaiters(new_status, WaitConnect);
    }
}

void MariaDbConn::beginQuery(const QByteArray &query)
{
    cur_query=query;
    int err=0;
    int new_status=mysql_real_query_start(&err, conn, cur_query.data(), cur_query.size());
    processQuery(new_status, err);
}

void MariaDbConn::continueQuery(int status)
{
    int err=0;
    int new_status=mysql_real_query_cont(&err, conn, status);
    processQuery(new_status, err);
}

void MariaDbConn::processQuery(int new_status, int err)
{
    if(new_status==0)
    {
        if(err==0)
        {
            if(mysql_field_count(conn)>0)
            {
                beginStoreResult();
            }
            else
            {
                onResult(MariaDbResult(conn, 0));
            }
        }
        else
        {
            onResult(MariaDbResult(conn, 0));
        }
    }
    else
    {
        setWaiters(new_status, WaitQuery);
    }
}

void MariaDbConn::beginStoreResult()
{
    MYSQL_RES *res;
    int new_status=mysql_store_result_start(&res, conn);
    processStoreResult(new_status, res);
}

void MariaDbConn::continueStoreResult(int status)
{
    MYSQL_RES *res;
    int new_status=mysql_store_result_cont(&res, conn, status);
    processStoreResult(new_status, res);
}

void MariaDbConn::processStoreResult(int new_status, MYSQL_RES *res)
{
    if(new_status==0)
    {
        if(res)
        {
            onResult(MariaDbResult(conn, res));
        }
        else
        {
            onResult(MariaDbResult(conn, 0));
        }
    }
    else
    {
        setWaiters(new_status, WaitResult);
    }
}

void MariaDbConn::processReady(int status)
{
    rn->setEnabled(false);
    wn->setEnabled(false);
    en->setEnabled(false);
    tmr.stop();

    switch(wait_kind)
    {
    case WaitConnect:
        continueConnect(status);
        break;
    case WaitQuery:
        continueQuery(status);
        break;
    case WaitResult:
        continueStoreResult(status);
        break;
    }
}

void MariaDbConn::setWaiters(int new_status, WaitKinds wait_kind)
{
    this->wait_kind=wait_kind;

    int sock=mysql_get_socket(conn);
    Q_ASSERT(sock>=0);
    if(cur_sock!=sock)
    {
        if(cur_sock>=0)
        {
            delete rn;
            rn=0;

            delete wn;
            wn=0;

            delete en;
            en=0;
        }

        rn=new QSocketNotifier(sock, QSocketNotifier::Read, this);
        connect(rn, SIGNAL(activated(int)), SLOT(onRead()));

        wn=new QSocketNotifier(sock, QSocketNotifier::Write, this);
        connect(wn, SIGNAL(activated(int)), SLOT(onWrite()));

        en=new QSocketNotifier(sock, QSocketNotifier::Exception, this);
        connect(en, SIGNAL(activated(int)), SLOT(onExcept()));

        cur_sock=sock;
    }

    rn->setEnabled(new_status&MYSQL_WAIT_READ);
    wn->setEnabled(new_status&MYSQL_WAIT_WRITE);
    en->setEnabled(new_status&MYSQL_WAIT_EXCEPT);
    if(new_status&MYSQL_WAIT_TIMEOUT) tmr.start(mysql_get_timeout_value_ms(conn));
    else tmr.stop();
}

void MariaDbConn::onRead()
{
    processReady(MYSQL_WAIT_READ);
}

void MariaDbConn::onWrite()
{
    processReady(MYSQL_WAIT_WRITE);
}

void MariaDbConn::onExcept()
{
    processReady(MYSQL_WAIT_EXCEPT);
}

void MariaDbConn::onTimeout()
{
    processReady(MYSQL_WAIT_TIMEOUT);
}
