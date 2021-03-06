#ifndef SQLERRORRESULTS_H
#define SQLERRORRESULTS_H

#include "sqlquery.h"
#include <QStringList>

/**
 * @brief SqlResults implementation for returning errors.
 *
 * It's very simple implementation of SqlResults, which has hardcoded number of columns and rows (0).
 * It has single constructor which accepts error code and error message, which are later
 * returned from getErrorCode() and getErrorText().
 */
class SqlErrorResults : public SqlQuery
{
    public:
        /**
         * @brief Creates error results with given code and message.
         * @param code Error code.
         * @param text Error message.
         */
        SqlErrorResults(int code, const QString &text);

        QString getErrorText();
        int getErrorCode();
        QStringList getColumnNames();
        int columnCount();
        qint64 rowsAffected();

    protected:
        SqlResultsRowPtr nextInternal();
        bool hasNextInternal();
        bool execInternal(const QList<QVariant>& args);
        bool execInternal(const QHash<QString, QVariant>& args);

    private:
        /**
         * @brief Error message passed in constructor.
         */
        QString errText;

        /**
         * @brief errCode Error code passed in constructor.
         */
        int errCode;
};

#endif // SQLERRORRESULTS_H
