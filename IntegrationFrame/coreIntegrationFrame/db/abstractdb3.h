#ifndef ABSTRACTDB3_H
#define ABSTRACTDB3_H

#include "db/abstractdb.h"
#include "common/unused.h"
#include "IntegrationFrame.h"
#include "db/sqlerrorcodes.h"
#include "common/utils_sql.h"
#include "log.h"
#include <QThread>
#include <QPointer>
#include <QDebug>

/**
 * @brief Complete implementation of SQLite 3 driver for SQLiteStudio.
 *
 * Inherit this when implementing Db for SQLite 3. In most cases you will only need
 * to create one public constructor, which forwards parameters to the AbstractDb constructor.
 * This be sufficient to implement SQLite 3 database plugin.
 * Just link it with proper SQLite library.
 *
 * The template parameter should provide all necessary SQLite symbols used by this implementation.
 * This way every Db plugin can provide it's own symbols to work on SQLite and so it allows
 * for loading multiple SQLite libraries into the same application, while symbols in each library
 * can be different (and should be different, to avoid name conflicts and symbol overlapping).
 * See how it's done in dbsqlite3.h.
 *
 * @see DbQt
 */
template <class T>
class AbstractDb3 : public AbstractDb
{
    public:
        /**
         * @brief Creates SQLite database object.
         * @param name Name for the database.
         * @param path File path of the database.
         * @param connOptions Connection options. See AbstractDb for details.
         *
         * All values from this constructor are just passed to AbstractDb constructor.
         */
        AbstractDb3(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions);
        ~AbstractDb3();

    protected:
        bool isOpenInternal();
        void interruptExecution();
        QString getErrorTextInternal();
        int getErrorCodeInternal();
        bool openInternal();
        bool closeInternal();
        bool initAfterCreated();
        void initAfterOpen();
        SqlQueryPtr prepare(const QString& query);
        QString getTypeLabel();

    private:
        class Query : public SqlQuery
        {
            public:
                class Row : public SqlResultsRow
                {
                    public:
                        int init(const QStringList& columns, typename T::stmt* stmt);

                    private:
                        int getValue(typename T::stmt* stmt, int col, QVariant& value);
                };

                Query(AbstractDb3<T>* db, const QString& query);
                ~Query();

                QString getErrorText();
                int getErrorCode();
                QStringList getColumnNames();
                int columnCount();
                qint64 rowsAffected();
                void finalize();

            protected:
                SqlResultsRowPtr nextInternal();
                bool hasNextInternal();
                bool execInternal(const QList<QVariant>& args);
                bool execInternal(const QHash<QString, QVariant>& args);

            private:
                int prepareStmt();
                int resetStmt();
                int bindParam(int paramIdx, const QVariant& value);
                int fetchFirst();
                int fetchNext();
                bool checkDbState();
                void copyErrorFromDb();
                void copyErrorToDb();
                void setError(int code, const QString& msg);

                QPointer<AbstractDb3<T>> db;
                typename T::stmt* stmt = nullptr;
                int errorCode = T::OK;
                QString errorMessage;
                int colCount = 0;
                QStringList colNames;
                bool rowAvailable = false;
        };

        struct CollationUserData
        {
            QString name;
            AbstractDb3<T>* db = nullptr;
        };

        QString extractLastError();
        void cleanUp();
        void resetError();

        /**
         * @brief Stores given result in function's context.
         * @param context Custom SQL function call context.
         * @param result Value returned from function execution.
         * @param ok true if the result is from a successful execution, or false if the result contains error message (QString).
         *
         * This method is called after custom implementation of the function was evaluated and it returned the result.
         * It stores the result in function's context, so it becomes the result of the function call.
         */
        static void storeResult(typename T::context* context, const QVariant& result, bool ok);

        /**
         * @brief Converts SQLite arguments into the list of argument values.
         * @param argCount Number of arguments.
         * @param args SQLite argument values.
         * @return Convenient Qt list with argument values as QVariant.
         *
         * This function does necessary conversions reflecting internal SQLite datatype, so if the type
         * was for example BLOB, then the QVariant will be a QByteArray, etc.
         */
        static QList<QVariant> getArgs(int argCount, typename T::value** args);

        /**
         * @brief Cleans up collation user data when collation is deregistered.
         * @param userData User data to delete.
         */
        static void deleteCollationUserData(void* userData);

        /**
         * @brief Destructor for function user data object.
         * @param dataPtr Pointer to the user data object.
         *
         * This is called by SQLite when the function is deregistered.
         */
        static void deleteUserData(void* dataPtr);

        /**
         * @brief Allocates and/or returns shared memory for the aggregate SQL function call.
         * @param context SQL function call context.
         * @return Pointer to the memory.
         *
         * It allocates exactly the number of bytes required to store pointer to a QHash.
         * The memory is released after the aggregate function is finished.
         */
        static void* getContextMemPtr(typename T::context* context);

        typename T::handle* dbHandle = nullptr;
        QString dbErrorMessage;
        int dbErrorCode = T::OK;
        QList<Query*> queries;

        /**
         * @brief User data for default collation request handling function.
         *
         * That function doesn't have destructor function pointer, so we need to keep track of that user data
         * and delete it when database is closed.
         */
        CollationUserData* defaultCollationUserData = nullptr;
};

//------------------------------------------------------------------------------------
// AbstractDb3
//------------------------------------------------------------------------------------

template <class T>
AbstractDb3<T>::AbstractDb3(const QString& name, const QString& path, const QHash<QString, QVariant>& connOptions) :
    AbstractDb(name, path, connOptions)
{
}

template <class T>
AbstractDb3<T>::~AbstractDb3()
{
    if (isOpenInternal())
        closeInternal();
}

template <class T>
bool AbstractDb3<T>::isOpenInternal()
{
    return dbHandle != nullptr;
}

template <class T>
void AbstractDb3<T>::interruptExecution()
{
    if (!isOpenInternal())
        return;

    T::interrupt(dbHandle);
}

template <class T>
QString AbstractDb3<T>::getErrorTextInternal()
{
    return dbErrorMessage;
}

template <class T>
int AbstractDb3<T>::getErrorCodeInternal()
{
    return dbErrorCode;
}

template <class T>
bool AbstractDb3<T>::openInternal()
{
    resetError();
    typename T::handle* handle = nullptr;
    int res = T::open_v2(path.toUtf8().constData(), &handle, T::OPEN_READWRITE|T::OPEN_CREATE, nullptr);
    if (res != T::OK)
    {
        if (handle)
            T::close(handle);

        dbErrorMessage = QObject::tr("Could not open database: %1").arg(extractLastError());
        dbErrorCode = res;
        return false;
    }
    dbHandle = handle;
    return true;
}

template <class T>
bool AbstractDb3<T>::closeInternal()
{
    resetError();
    if (!dbHandle)
        return false;

    cleanUp();

    int res = T::close(dbHandle);
    if (res != T::OK)
    {
        dbErrorMessage = QObject::tr("Could not close database: %1").arg(extractLastError());
        dbErrorCode = res;
        qWarning() << "Error closing database. That's weird:" << dbErrorMessage;
        return false;
    }
    dbHandle = nullptr;
    return true;
}

template <class T>
bool AbstractDb3<T>::initAfterCreated()
{
    version = 3;
    return AbstractDb::initAfterCreated();
}

template <class T>
void AbstractDb3<T>::initAfterOpen()
{
    T::enable_load_extension(dbHandle, true);
    //registerDefaultCollationRequestHandler();;
    exec("PRAGMA foreign_keys = 1;", Flag::NO_LOCK);
    exec("PRAGMA recursive_triggers = 1;", Flag::NO_LOCK);
}

template <class T>
SqlQueryPtr AbstractDb3<T>::prepare(const QString& query)
{
    return SqlQueryPtr(new Query(this, query));
}

template <class T>
QString AbstractDb3<T>::getTypeLabel()
{
    return T::label;
}


template <class T>
QString AbstractDb3<T>::extractLastError()
{
    dbErrorCode = T::extended_errcode(dbHandle);
    dbErrorMessage = QString::fromUtf8(T::errmsg(dbHandle));
    return dbErrorMessage;
}

template <class T>
void AbstractDb3<T>::cleanUp()
{
    for (Query* q : queries)
        q->finalize();

    safe_delete(defaultCollationUserData);
}

template <class T>
void AbstractDb3<T>::resetError()
{
    dbErrorCode = 0;
    dbErrorMessage = QString::null;
}

template <class T>
void AbstractDb3<T>::storeResult(typename T::context* context, const QVariant& result, bool ok)
{
    if (!ok)
    {
        QString str = result.toString();
        T::result_error16(context, str.utf16(), str.size() * sizeof(QChar));
        return;
    }

    // Code below is a modified code from Qt (its SQLite plugin).
    if (result.isNull())
    {
        T::result_null(context);
        return;
    }

    switch (result.type())
    {
        case QVariant::ByteArray:
        {
            QByteArray ba = result.toByteArray();
            T::result_blob(context, ba.constData(), ba.size(), T::TRANSIENT());
            break;
        }
        case QVariant::Int:
        case QVariant::Bool:
        {
            T::result_int(context, result.toInt());
            break;
        }
        case QVariant::Double:
        {
            T::result_double(context, result.toDouble());
            break;
        }
        case QVariant::UInt:
        case QVariant::LongLong:
        {
            T::result_int64(context, result.toLongLong());
            break;
        }
        case QVariant::List:
        {
            QList<QVariant> list = result.toList();
            QStringList strList;
            for (const QVariant& v : list)
                strList << v.toString();

            QString str = strList.join(" ");
            T::result_text16(context, str.utf16(), str.size() * sizeof(QChar), T::TRANSIENT());
            break;
        }
        case QVariant::StringList:
        {
            QString str = result.toStringList().join(" ");
            T::result_text16(context, str.utf16(), str.size() * sizeof(QChar), T::TRANSIENT());
            break;
        }
        default:
        {
            // T::TRANSIENT makes sure that sqlite buffers the data
            QString str = result.toString();
            T::result_text16(context, str.utf16(), str.size() * sizeof(QChar), T::TRANSIENT());
            break;
        }
    }
}

template <class T>
QList<QVariant> AbstractDb3<T>::getArgs(int argCount, typename T::value** args)
{
    int dataType;
    QList<QVariant> results;
    QVariant value;

    // The loop below uses slightly modified code from Qt (its SQLite plugin) to extract values.
    for (int i = 0; i < argCount; i++)
    {
        dataType = T::value_type(args[i]);
        switch (dataType)
        {
            case T::INTEGER:
                value = T::value_int64(args[i]);
                break;
            case T::BLOB:
                value = QByteArray(
                            static_cast<const char*>(T::value_blob(args[i])),
                            T::value_bytes(args[i])
                            );
                break;
            case T::FLOAT:
                value = T::value_double(args[i]);
                break;
            case T::NULL_TYPE:
                value = QVariant(QVariant::String);
                break;
            default:
                value = QString(
                            reinterpret_cast<const QChar*>(T::value_text16(args[i])),
                            T::value_bytes16(args[i]) / sizeof(QChar)
                            );
                break;
        }
        results << value;
    }
    return results;
}

template <class T>
void AbstractDb3<T>::deleteCollationUserData(void* userData)
{
    if (!userData)
        return;

    CollationUserData* collUserData = reinterpret_cast<CollationUserData*>(userData);
    delete collUserData;
}

template <class T>
void AbstractDb3<T>::deleteUserData(void* dataPtr)
{
    if (!dataPtr)
        return;

    FunctionUserData* userData = reinterpret_cast<FunctionUserData*>(dataPtr);
    delete userData;
}

template <class T>
void* AbstractDb3<T>::getContextMemPtr(typename T::context* context)
{
    return T::aggregate_context(context, sizeof(QHash<QString,QVariant>**));
}

//------------------------------------------------------------------------------------
// Results
//------------------------------------------------------------------------------------

template <class T>
AbstractDb3<T>::Query::Query(AbstractDb3<T>* db, const QString& query) :
    db(db)
{
    this->query = query;
    db->queries << this;
}

template <class T>
AbstractDb3<T>::Query::~Query()
{
    if (db.isNull())
        return;

    finalize();
    db->queries.removeOne(this);
}

template <class T>
void AbstractDb3<T>::Query::copyErrorFromDb()
{
    if (db->dbErrorCode != 0)
    {
        errorCode = db->dbErrorCode;
        errorMessage = db->dbErrorMessage;
        return;
    }
}

template <class T>
void AbstractDb3<T>::Query::copyErrorToDb()
{
    db->dbErrorCode = errorCode;
    db->dbErrorMessage = errorMessage;
}

template <class T>
void AbstractDb3<T>::Query::setError(int code, const QString& msg)
{
    if (errorCode != T::OK)
        return; // don't overwrite first error

    errorCode = code;
    errorMessage = msg;
    copyErrorToDb();
}

template <class T>
int AbstractDb3<T>::Query::prepareStmt()
{
    const char* tail;
    QByteArray queryBytes = query.toUtf8();
    int res = T::prepare_v2(db->dbHandle, queryBytes.constData(), queryBytes.size(), &stmt, &tail);
    if (res != T::OK)
    {
        stmt = nullptr;
        db->extractLastError();
        copyErrorFromDb();
        return res;
    }

    if (tail && !QString::fromUtf8(tail).trimmed().isEmpty())
        qWarning() << "Executed query left with tailing contents:" << tail << ", while executing query:" << query;

    return T::OK;
}

template <class T>
int AbstractDb3<T>::Query::resetStmt()
{
    errorCode = 0;
    errorMessage = QString::null;
    affected = 0;
    colCount = -1;
    rowAvailable = false;

    int res = T::reset(stmt);
    if (res != T::OK)
    {
        stmt = nullptr;
        setError(res, QString::fromUtf8(T::errmsg(db->dbHandle)));
        return res;
    }
    return T::OK;
}

template <class T>
bool AbstractDb3<T>::Query::execInternal(const QList<QVariant>& args)
{
    if (!checkDbState())
        return false;

    ReadWriteLocker locker(&(db->dbOperLock), query, Dialect::Sqlite3, flags.testFlag(Db::Flag::NO_LOCK));
    logSql(db.data(), query, args, flags);

    QueryWithParamCount queryWithParams = getQueryWithParamCount(query, Dialect::Sqlite3);

    int res;
    if (stmt)
        res = resetStmt();
    else
        res = prepareStmt();

    if (res != T::OK)
        return false;


    for (int paramIdx = 1, argCount = args.size(); paramIdx <= queryWithParams.second && paramIdx <= argCount; paramIdx++)
    {
        res = bindParam(paramIdx, args[paramIdx-1]);
        if (res != T::OK)
        {
            db->extractLastError();
            copyErrorFromDb();
            return false;
        }
    }

    bool ok = (fetchFirst() == T::OK);
    if (ok)
        db->checkForDroppedObject(query);

    return ok;
}

template <class T>
bool AbstractDb3<T>::Query::execInternal(const QHash<QString, QVariant>& args)
{
    if (!checkDbState())
        return false;

    ReadWriteLocker locker(&(db->dbOperLock), query, Dialect::Sqlite3, flags.testFlag(Db::Flag::NO_LOCK));
    logSql(db.data(), query, args, flags);

    QueryWithParamNames queryWithParams = getQueryWithParamNames(query, Dialect::Sqlite3);

    int res;
    if (stmt)
        res = resetStmt();
    else
        res = prepareStmt();

    if (res != T::OK)
        return false;

    int paramIdx = -1;
    for (const QString& paramName : queryWithParams.second)
    {
        if (!args.contains(paramName))
        {
            qWarning() << "Could not bind parameter" << paramName << "because it was not found in passed arguments.";
            setError(SqlErrorCode::OTHER_EXECUTION_ERROR, "Error while preparing statement: could not bind parameter " + paramName);
            return false;
        }

        paramIdx = T::bind_parameter_index(stmt, paramName.toUtf8().constData());
        if (paramIdx <= 0)
        {
            qWarning() << "Could not bind parameter" << paramName << "because it was not found in prepared statement.";
            setError(SqlErrorCode::OTHER_EXECUTION_ERROR, "Error while preparing statement: could not bind parameter " + paramName);
            return false;
        }

        res = bindParam(paramIdx, args[paramName]);
        if (res != T::OK)
        {
            db->extractLastError();
            copyErrorFromDb();
            return false;
        }
    }

    bool ok = (fetchFirst() == T::OK);
    if (ok)
        db->checkForDroppedObject(query);

    return ok;
}

template <class T>
int AbstractDb3<T>::Query::bindParam(int paramIdx, const QVariant& value)
{
    if (value.isNull())
    {
        return T::bind_null(stmt, paramIdx);
    }

    switch (value.type())
    {
        case QVariant::ByteArray:
        {
            QByteArray ba = value.toByteArray();
            return T::bind_blob(stmt, paramIdx, ba.constData(), ba.size(), T::TRANSIENT());
        }
        case QVariant::Int:
        case QVariant::Bool:
        {
            return T::bind_int(stmt, paramIdx, value.toInt());
        }
        case QVariant::Double:
        {
            return T::bind_double(stmt, paramIdx, value.toDouble());
        }
        case QVariant::UInt:
        case QVariant::LongLong:
        {
            return T::bind_int64(stmt, paramIdx, value.toLongLong());
        }
        default:
        {
            // T::TRANSIENT makes sure that sqlite buffers the data
            QString str = value.toString();
            return T::bind_text16(stmt, paramIdx, str.utf16(), str.size() * sizeof(QChar), T::TRANSIENT());
        }
    }

    return T::MISUSE; // not going to happen
}

template <class T>
bool AbstractDb3<T>::Query::checkDbState()
{
    if (db.isNull() || !db->dbHandle)
    {
        setError(SqlErrorCode::DB_NOT_DEFINED, "SqlQuery is no longer valid.");
        return false;
    }

    return true;
}

template <class T>
void AbstractDb3<T>::Query::finalize()
{
    if (stmt)
    {
        T::finalize(stmt);
        stmt = nullptr;
    }
}

template <class T>
QString AbstractDb3<T>::Query::getErrorText()
{
    return errorMessage;
}

template <class T>
int AbstractDb3<T>::Query::getErrorCode()
{
    return errorCode;
}

template <class T>
QStringList AbstractDb3<T>::Query::getColumnNames()
{
    return colNames;
}

template <class T>
int AbstractDb3<T>::Query::columnCount()
{
    return colCount;
}

template <class T>
qint64 AbstractDb3<T>::Query::rowsAffected()
{
    return affected;
}

template <class T>
SqlResultsRowPtr AbstractDb3<T>::Query::nextInternal()
{
    Row* row = new Row;
    int res = row->init(colNames, stmt);
    if (res != T::OK)
    {
        delete row;
        setError(res, QString::fromUtf8(T::errmsg(db->dbHandle)));
        return SqlResultsRowPtr();
    }

    res = fetchNext();
    if (res != T::OK)
    {
        delete row;
        return SqlResultsRowPtr();
    }

    return SqlResultsRowPtr(row);
}

template <class T>
bool AbstractDb3<T>::Query::hasNextInternal()
{
    return rowAvailable && stmt && checkDbState();
}

template <class T>
int AbstractDb3<T>::Query::fetchFirst()
{
    colCount = T::column_count(stmt);
    for (int i = 0; i < colCount; i++)
        colNames << QString::fromUtf8(T::column_name(stmt, i));

    int changesBefore =  T::total_changes(db->dbHandle);
    rowAvailable = true;
    int res = fetchNext();

    affected = 0;
    if (res == T::OK)
    {
        affected =  T::total_changes(db->dbHandle) - changesBefore;
        insertRowId["ROWID"] = T::last_insert_rowid(db->dbHandle);
    }

    return res;
}

template <class T>
int AbstractDb3<T>::Query::fetchNext()
{
    if (!checkDbState())
        rowAvailable = false;

    if (!rowAvailable || !stmt)
    {
        setError(T::MISUSE, QObject::tr("Result set expired or no row available."));
        return T::MISUSE;
    }

    rowAvailable = false;
    int res;
    int secondsSpent = 0;
    while ((res = T::step(stmt)) == T::BUSY && secondsSpent < db->getTimeout())
    {
        QThread::sleep(1);
        if (db->getTimeout() >= 0)
            secondsSpent++;
    }

    switch (res)
    {
        case T::ROW:
            rowAvailable = true;
            break;
        case T::DONE:
            // Empty pointer as no more results are available.
            break;
        default:
            setError(res, QString::fromUtf8(T::errmsg(db->dbHandle)));
            return T::ERROR;
    }
    return T::OK;
}

//------------------------------------------------------------------------------------
// Row
//------------------------------------------------------------------------------------

template <class T>
int AbstractDb3<T>::Query::Row::init(const QStringList& columns, typename T::stmt* stmt)
{
    int res = T::OK;
    QVariant value;
    for (int i = 0; i < columns.size(); i++)
    {
        res = getValue(stmt, i, value);
        if (res != T::OK)
            return res;

        values << value;
        valuesMap[columns[i]] = value;
    }
    return res;
}

template <class T>
int AbstractDb3<T>::Query::Row::getValue(typename T::stmt* stmt, int col, QVariant& value)
{
    int dataType = T::column_type(stmt, col);
    switch (dataType)
    {
        case T::INTEGER:
            value = T::column_int64(stmt, col);
            break;
        case T::BLOB:
            value = QByteArray(
                        static_cast<const char*>(T::column_blob(stmt, col)),
                        T::column_bytes(stmt, col)
                        );
            break;
        case T::FLOAT:
            value = T::column_double(stmt, col);
            break;
        case T::NULL_TYPE:
            value = QVariant(QVariant::String);
            break;
        default:
            value = QString(
                            reinterpret_cast<const QChar*>(T::column_text16(stmt, col)),
                            T::column_bytes16(stmt, col) / sizeof(QChar)
                        );
            break;
    }
    return T::OK;
}

#endif // ABSTRACTDB3_H
