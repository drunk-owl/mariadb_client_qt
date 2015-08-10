#ifndef MARIADB_RESULT_H
#define MARIADB_RESULT_H

#include <mysql/mysql.h>
#include <QVariant>
#include <QMetaType>
#include <QExplicitlySharedDataPointer>
#include <QSharedData>

class MariaDbResultData : public QSharedData
{
public:
    MariaDbResultData(MYSQL *conn, MYSQL_RES *result);
    MariaDbResultData(const MariaDbResultData&);
    MariaDbResultData();
    ~MariaDbResultData();

    MYSQL_RES *result;

    unsigned error_code;
    QByteArray sql_state;
    QString error_str;

    unsigned long long last_insert_id, affected_rows;
    unsigned field_count, row_count;

    MYSQL_ROW cur_row;
    unsigned long *cur_row_lengths;
};

class MariaDbResult
{
public:
    MariaDbResult(MYSQL *conn, MYSQL_RES *result);
    MariaDbResult(const MariaDbResult &oth);
    MariaDbResult();

    unsigned error() const;
    QByteArray sqlState() const;
    QString errorString() const;

    unsigned long long lastInsertId() const;
    unsigned long long affectedRows() const;

    unsigned rowCount() const;
    unsigned fieldCount() const;
    bool next();
    QVariant value(unsigned col) const;

private:
    QVariant shortValue(const char *data, unsigned length, bool u) const;
    QVariant intValue(const char *data, unsigned length, bool u) const;
    QVariant longLongValue(const char *data, unsigned length, bool u) const;
    QVariant doubleValue(const char *data, unsigned length) const;

    QVariant dateTimeValue(const char *data, unsigned length) const;
    QVariant dateValue(const char *data, unsigned length) const;
    QVariant timeValue(const char *data, unsigned length) const;

    QVariant stringValue(const char *data, unsigned length, bool b) const;

    QExplicitlySharedDataPointer<MariaDbResultData> d;
};

#endif // MARIADB_RESULT_H
