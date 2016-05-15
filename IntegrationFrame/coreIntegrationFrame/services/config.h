#ifndef CONFIG_H
#define CONFIG_H

#include "coreIntegrationFrame_global.h"
#include "config_builder.h"
#include "IntegrationFrame.h"
#include "common/utils.h"
#include <QObject>
#include <QVariant>
#include <QHash>
#include <QStringList>
#include <QSharedPointer>
#include <QDateTime>

const int SQLITESTUDIO_CONFIG_VERSION = 1;

//TODO 根据实际需要进行定制
CFG_CATEGORIES(Core,
    CFG_CATEGORY(General,
        CFG_ENTRY(int,          SqlHistorySize,          10000)
        CFG_ENTRY(int,          DdlHistorySize,          1000)
        CFG_ENTRY(QString,      LoadedPlugins,           "")
        CFG_ENTRY(QVariantHash, ActiveCodeFormatter,     QVariantHash())
        CFG_ENTRY(bool,         CheckUpdatesOnStartup,   true)
        CFG_ENTRY(QString,      Language,                "en")
    )
    CFG_CATEGORY(Console,
        CFG_ENTRY(int,          HistorySize,             100)
    )
    CFG_CATEGORY(Internal,
        CFG_ENTRY(QVariantList, Functions,               QVariantList())
        CFG_ENTRY(QVariantList, Collations,              QVariantList())
        CFG_ENTRY(QString,      BugReportUser,           QString())
        CFG_ENTRY(QString,      BugReportPassword,       QString())
        CFG_ENTRY(QString,      BugReportRecentTitle,    QString())
        CFG_ENTRY(QString,      BugReportRecentContents, QString())
        CFG_ENTRY(bool,         BugReportRecentError,    false)
    )
)

#define CFG_CORE CFG_INSTANCE(Core)

class QAbstractItemModel;
class DdlHistoryModel;

class API_EXPORT Config : public QObject
{
    Q_OBJECT

    public:
        virtual ~Config();

        struct CfgDb
        {
            QString name;
            QString path;
            QHash<QString,QVariant> options;
        };

        typedef QSharedPointer<CfgDb> CfgDbPtr;

        virtual void init() = 0;
        virtual void cleanUp() = 0;
        virtual const QString& getConfigDir() const = 0;
        virtual QString getConfigFilePath() const = 0;

        virtual void beginMassSave() = 0;
        virtual void commitMassSave() = 0;
        virtual void rollbackMassSave() = 0;
        virtual bool isMassSaving() const = 0;
        virtual void set(const QString& group, const QString& key, const QVariant& value) = 0;
        virtual QVariant get(const QString& group, const QString& key) = 0;
        virtual QHash<QString,QVariant> getAll() = 0;

        virtual bool addDb(const QString& name, const QString& path, const QHash<QString, QVariant> &options) = 0;
        virtual bool updateDb(const QString& name, const QString &newName, const QString& path, const QHash<QString, QVariant> &options) = 0;
        virtual bool removeDb(const QString& name) = 0;
        virtual bool isDbInConfig(const QString& name) = 0;
        virtual QString getLastErrorString() const = 0;

        /**
         * @brief Provides list of all registered databases.
         * @return List of database entries.
         *
         * Registered databases are those that user added to the application. They are not necessary valid or supported.
         * They can be inexisting or unsupported, but they are kept in registry in case user fixes file path,
         * or loads plugin to support it.
         */
        virtual QList<CfgDbPtr> dbList() = 0;
        virtual CfgDbPtr getDb(const QString& dbName) = 0;

        virtual void begin() = 0;
        virtual void commit() = 0;
        virtual void rollback() = 0;

        virtual QString getSqlite3Version() const = 0;

    signals:
        void massSaveBegins();
        void massSaveCommited();
};

#define CFG INTEGRATIONFRAME->getConfig()

#endif // CONFIG_H
