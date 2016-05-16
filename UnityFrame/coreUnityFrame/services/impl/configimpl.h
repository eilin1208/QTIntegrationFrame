#ifndef CONFIGIMPL_H
#define CONFIGIMPL_H

#include "coreUnityFrame_global.h"
#include "services/config.h"
#include "db/sqlquery.h"
#include <QMutex>

class AsyncConfigHandler;
class SqlHistoryModel;

class API_EXPORT ConfigImpl : public Config
{
    Q_OBJECT

    friend class AsyncConfigHandler;

    public:
        virtual ~ConfigImpl();

        void init();
        void cleanUp();
        const QString& getConfigDir() const;
        QString getConfigFilePath() const;

        void beginMassSave();
        void commitMassSave();
        void rollbackMassSave();
        bool isMassSaving() const;
        void set(const QString& group, const QString& key, const QVariant& value);
        QVariant get(const QString& group, const QString& key);
        QHash<QString,QVariant> getAll();

        bool addDb(const QString& name, const QString& path, const QHash<QString, QVariant> &options);
        bool updateDb(const QString& name, const QString &newName, const QString& path, const QHash<QString, QVariant> &options);
        bool removeDb(const QString& name);
        bool isDbInConfig(const QString& name);
        QString getLastErrorString() const;
        QString getSqlite3Version() const;

        /**
         * @brief Provides list of all registered databases.
         * @return List of database entries.
         *
         * Registered databases are those that user added to the application. They are not necessary valid or supported.
         * They can be inexisting or unsupported, but they are kept in registry in case user fixes file path,
         * or loads plugin to support it.
         */
        QList<CfgDbPtr> dbList();
        CfgDbPtr getDb(const QString& dbName);

        void begin();
        void commit();
        void rollback();

    private:
        /**
         * @brief Stores error from query in class member.
         * @param query Query to get error from.
         * @return true if the query had any error set, or false if not.
         *
         * If the error was defined in the query, its message is stored in lastQueryError.
         */
        bool storeErrorAndReturn(SqlQueryPtr results);
        void printErrorIfSet(SqlQueryPtr results);
        QString getConfigPath();
        QString getPortableConfigPath();
        void initTables();
        void initDbFile();
        bool tryInitDbFile(const QPair<QString, bool>& dbPath);
        QVariant deserializeValue(const QVariant& value);

        static Config* instance;
        static qint64 sqlHistoryId;

        Db* db = nullptr;
        QString configDir;
        QString lastQueryError;
        bool massSaving = false;
        SqlHistoryModel* sqlHistoryModel = nullptr;
        DdlHistoryModel* ddlHistoryModel = nullptr;
        QMutex sqlHistoryMutex;
        QString sqlite3Version;

};

#endif // CONFIGIMPL_H
