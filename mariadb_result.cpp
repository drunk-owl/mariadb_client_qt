#include "mariadb_result.h"
#include <QDateTime>

const int type=qRegisterMetaType<MariaDbResult>("MariaDbResult");

MariaDbResultData::MariaDbResultData(MYSQL *conn, MYSQL_RES *result) :
    QSharedData(),
    result(result),
    last_insert_id(0), affected_rows(0),
    field_count(0), row_count(0),
    cur_row(0), cur_row_lengths(0)
{
    error_code=mysql_errno(conn);

    if(error_code!=0)
    {
        sql_state=mysql_sqlstate(conn);
        error_str=QString::fromUtf8(mysql_error(conn));
    }
    else
    {
        sql_state="00000";
        error_str="No error";

        if(result!=0)//SELECT
        {
            field_count=mysql_field_count(conn);
            row_count=mysql_num_rows(result);
        }
        else
        {
            last_insert_id=mysql_insert_id(conn);
            affected_rows=mysql_affected_rows(conn);
        }
    }
}

MariaDbResultData::MariaDbResultData(const MariaDbResultData &) :
    QSharedData()
{
    Q_ASSERT_X(false, __FUNCTION__, "Deep copy of MariaDbResultData is forbidden");
}

MariaDbResultData::MariaDbResultData() :
    QSharedData(),
    result(0),
    error_code(0), sql_state("00000"), error_str("No error"),
    last_insert_id(0), affected_rows(0),
    field_count(0), row_count(0),
    cur_row(0), cur_row_lengths(0)
{
}

MariaDbResultData::~MariaDbResultData()
{
    if(result!=0) mysql_free_result(result);
}


MariaDbResult::MariaDbResult(MYSQL *conn, MYSQL_RES *result) :
    d(new MariaDbResultData(conn, result))
{
}

MariaDbResult::MariaDbResult(const MariaDbResult &oth) :
    d(oth.d)
{
}

MariaDbResult::MariaDbResult() :
    d(new MariaDbResultData())
{
}

unsigned MariaDbResult::error() const
{
    return d->error_code;
}

QByteArray MariaDbResult::sqlState() const
{
    return d->sql_state;
}

QString MariaDbResult::errorString() const
{
    return d->error_str;
}

unsigned long long MariaDbResult::lastInsertId() const
{
    return d->last_insert_id;
}

unsigned long long MariaDbResult::affectedRows() const
{
    return d->affected_rows;
}

unsigned MariaDbResult::rowCount() const
{
    return d->row_count;
}

unsigned MariaDbResult::fieldCount() const
{
    return d->field_count;
}

bool MariaDbResult::next()
{
    Q_ASSERT(d->result!=0);
    d->cur_row=mysql_fetch_row(d->result);
    if(d->cur_row!=0)
    {
        d->cur_row_lengths=mysql_fetch_lengths(d->result);
        return true;
    }
    else
    {
        d->cur_row_lengths=0;
        return false;
    }
}

QVariant MariaDbResult::value(unsigned col) const
{
    Q_ASSERT(d->cur_row!=0 && col<d->field_count);

    MYSQL_FIELD *field=mysql_fetch_field_direct(d->result, col);
    switch(field->type)
    {
    case MYSQL_TYPE_NULL:
        return QVariant();

    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
        return shortValue(d->cur_row[col], d->cur_row_lengths[col], field->flags&UNSIGNED_FLAG);
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG:
        return intValue(d->cur_row[col], d->cur_row_lengths[col], field->flags&UNSIGNED_FLAG);
    case MYSQL_TYPE_LONGLONG:
        return longLongValue(d->cur_row[col], d->cur_row_lengths[col], field->flags&UNSIGNED_FLAG);

    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NEWDECIMAL:
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
        return doubleValue(d->cur_row[col], d->cur_row_lengths[col]);

    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
        return dateTimeValue(d->cur_row[col], d->cur_row_lengths[col]);
    case MYSQL_TYPE_DATE:
        return dateValue(d->cur_row[col], d->cur_row_lengths[col]);
    case MYSQL_TYPE_TIME:
        return timeValue(d->cur_row[col], d->cur_row_lengths[col]);

    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_SET:
        return stringValue(d->cur_row[col], d->cur_row_lengths[col], field->flags&BINARY_FLAG);

    default:
        Q_ASSERT_X(false, __FUNCTION__, QString("type %1 not implemented").arg(field->type).toLatin1().data());
    }

    return QVariant();
}

QVariant MariaDbResult::shortValue(const char *data, unsigned length, bool u) const
{
    if(u) return QByteArray(data, length).toUShort();
    else return QByteArray(data, length).toShort();
}

QVariant MariaDbResult::intValue(const char *data, unsigned length, bool u) const
{
    if(u) return QByteArray(data, length).toUInt();
    else return QByteArray(data, length).toInt();
}

QVariant MariaDbResult::longLongValue(const char *data, unsigned length, bool u) const
{
    if(u) return QByteArray(data, length).toULongLong();
    else return QByteArray(data, length).toLongLong();
}

QVariant MariaDbResult::doubleValue(const char *data, unsigned length) const
{
    return QByteArray(data, length).toDouble();
}

QVariant MariaDbResult::dateTimeValue(const char *data, unsigned length) const
{
    return QDateTime::fromString(QString::fromLatin1(data, length), "yyyy-MM-dd hh:mm:ss");
}

QVariant MariaDbResult::dateValue(const char *data, unsigned length) const
{
    return QDate::fromString(QString::fromLatin1(data, length), "yyyy-MM-dd");
}

QVariant MariaDbResult::timeValue(const char *data, unsigned length) const
{
    return QTime::fromString(QString::fromLatin1(data, length), "hh:mm:ss");
}

QVariant MariaDbResult::stringValue(const char *data, unsigned length, bool b) const
{
    if(b) return QByteArray(data, length);
    else return QString::fromUtf8(data, length);
}
