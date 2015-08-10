#include "mariadb_conn_block.h"

MariaDbConnBlock::MariaDbConnBlock() :
    conn(0)
{
}

MariaDbConnBlock::~MariaDbConnBlock()
{
    if(conn)
    {
        mysql_close(conn);
        conn=0;
    }
}

bool MariaDbConnBlock::connect(const MariaDbConnParams &conn_params)
{
    conn=mysql_init(0);

    mysql_options(conn, MYSQL_OPT_RECONNECT, (const void*)"1");
    if(conn_params.conn_timeout>0) mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, (const void*)&conn_params.conn_timeout);
    mysql_options(conn, MYSQL_SET_CHARSET_NAME, (const void*)"utf8");

    MYSQL *res=mysql_real_connect(conn,
                                  conn_params.host.data(),
                                  conn_params.user.data(),
                                  conn_params.password.data(),
                                  conn_params.db_name.data(),
                                  conn_params.port,
                                  0, 0);
    return (res!=0);
}

MariaDbResult MariaDbConnBlock::exec(const QByteArray &query)
{
    Q_ASSERT(conn!=0);

    if(mysql_real_query(conn, query.data(), query.size())!=0)
    {
        return MariaDbResult(conn, 0);
    }

    MYSQL_RES *res=0;
    if(mysql_field_count(conn)!=0)//SELECT
    {
        res=mysql_store_result(conn);
    }
    return MariaDbResult(conn, res);
}

MariaDbResult MariaDbConnBlock::exec(const QString &query)
{
    return exec(query.toUtf8());
}

QString MariaDbConnBlock::errorString() const
{
    return QString::fromUtf8(mysql_error(conn));
}
