#include "configimpl.h"
#include "services/notifymanager.h"
#include "UnityFrame.h"
#include "db/dbsqlite3.h"
#include <QtGlobal>
#include <QDebug>
#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QDataStream>
#include <QRegExp>
#include <QDateTime>
#include <QSysInfo>
#include <QCoreApplication>
#include <QtConcurrent/QtConcurrentRun>

static_qstring(DB_FILE_NAME, "settings3");
qint64 ConfigImpl::sqlHistoryId = -1;

ConfigImpl::~ConfigImpl()
{
    cleanUp();
}

void ConfigImpl::init()
{
    initDbFile();
    initTables();

    sqlite3Version = db->exec("SELECT sqlite_version()")->getSingleCell().toString();
}

void ConfigImpl::cleanUp()
{
    if (db->isOpen())
        db->close();

    safe_delete(db);
}

const QString &ConfigImpl::getConfigDir() const
{
    return configDir;
}

QString ConfigImpl::getConfigFilePath() const
{
    if (!db)
        return QString();

    return db->getPath();
}

void ConfigImpl::beginMassSave()
{
    if (isMassSaving())
        return;

    emit massSaveBegins();
    db->exec("BEGIN;");
    massSaving = true;
}

void ConfigImpl::commitMassSave()
{
    if (!isMassSaving())
        return;

    db->exec("COMMIT;");
    emit massSaveCommited();
    massSaving = false;
}

void ConfigImpl::rollbackMassSave()
{
    if (!isMassSaving())
        return;

    db->exec("ROLLBACK;");
    massSaving = false;
}

bool ConfigImpl::isMassSaving() const
{
    return massSaving;
}

void ConfigImpl::set(const QString &group, const QString &key, const QVariant &value)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << value;

    db->exec("INSERT OR REPLACE INTO settings VALUES (?, ?, ?)", {group, key, bytes});
}

QVariant ConfigImpl::get(const QString &group, const QString &key)
{
    SqlQueryPtr results = db->exec("SELECT value FROM settings WHERE [group] = ? AND [key] = ?", {group, key});
    return deserializeValue(results->getSingleCell());
}

QHash<QString,QVariant> ConfigImpl::getAll()
{
    SqlQueryPtr results = db->exec("SELECT [group], [key], value FROM settings");

    QHash<QString,QVariant> cfg;
    QString key;
    SqlResultsRowPtr row;
    while (results->hasNext())
    {
        row = results->next();
        key = row->value("group").toString() + "." + row->value("key").toString();
        cfg[key] = deserializeValue(row->value("value"));
    }
    return cfg;
}

bool ConfigImpl::storeErrorAndReturn(SqlQueryPtr results)
{
    if (results->isError())
    {
        lastQueryError = results->getErrorText();
        return true;
    }
    else
        return false;
}

void ConfigImpl::printErrorIfSet(SqlQueryPtr results)
{
    if (results && results->isError())
    {
        qCritical() << "Config error while executing query:" << results->getErrorText();
        storeErrorAndReturn(results);
    }
}

bool ConfigImpl::addDb(const QString& name, const QString& path, const QHash<QString,QVariant>& options)
{
    QByteArray optBytes = hashToBytes(options);
    SqlQueryPtr results = db->exec("INSERT INTO dblist VALUES (?, ?, ?)", {name, path, optBytes});
    return !storeErrorAndReturn(results);
}

bool ConfigImpl::updateDb(const QString &name, const QString &newName, const QString &path, const QHash<QString,QVariant> &options)
{
    QByteArray optBytes = hashToBytes(options);
    SqlQueryPtr results = db->exec("UPDATE dblist SET name = ?, path = ?, options = ? WHERE name = ?",
                                     {newName, path, optBytes, name});

    return (!storeErrorAndReturn(results)  && results->rowsAffected() > 0);
}

bool ConfigImpl::removeDb(const QString &name)
{
    SqlQueryPtr results = db->exec("DELETE FROM dblist WHERE name = ?", {name});
    return (!storeErrorAndReturn(results) && results->rowsAffected() > 0);
}

bool ConfigImpl::isDbInConfig(const QString &name)
{
    SqlQueryPtr results = db->exec("SELECT * FROM dblist WHERE name = ?", {name});
    return (!storeErrorAndReturn(results) && results->hasNext());
}

QString ConfigImpl::getLastErrorString() const
{
    QString msg = db->getErrorText();
    if (msg.trimmed().isEmpty())
        return lastQueryError;

    return msg;
}

QString ConfigImpl::getSqlite3Version() const
{
    return sqlite3Version;
}

QList<ConfigImpl::CfgDbPtr> ConfigImpl::dbList()
{
    QList<CfgDbPtr> entries;
    SqlQueryPtr results = db->exec("SELECT name, path, options FROM dblist");
    CfgDbPtr cfgDb;
    SqlResultsRowPtr row;
    while (results->hasNext())
    {
        row = results->next();
        cfgDb = CfgDbPtr::create();
        cfgDb->name = row->value("name").toString();
        cfgDb->path = row->value("path").toString();
        cfgDb->options = deserializeValue(row->value("options")).toHash();
        entries += cfgDb;
    }

    return entries;
}

ConfigImpl::CfgDbPtr ConfigImpl::getDb(const QString& dbName)
{
    SqlQueryPtr results = db->exec("SELECT path, options FROM dblist WHERE name = ?", {dbName});

    if (!results->hasNext())
        return CfgDbPtr();

    SqlResultsRowPtr row = results->next();

    CfgDbPtr cfgDb = CfgDbPtr::create();
    cfgDb->name = dbName;
    cfgDb->path = row->value("path").toString();
    cfgDb->options = deserializeValue(row->value("options")).toHash();
    return cfgDb;
}

void ConfigImpl::begin()
{
    db->begin();
}

void ConfigImpl::commit()
{
    db->commit();
}

void ConfigImpl::rollback()
{
    db->rollback();
}

QString ConfigImpl::getConfigPath()
{
#ifdef Q_OS_WIN
    if (QSysInfo::windowsVersion() & QSysInfo::WV_NT_based)
        return UNITYFRAME->getEnv("APPDATA")+"/sqlitestudio";
    else
        return UNITYFRAME->getEnv("HOME")+"/sqlitestudio";
#else
    return UNITYFRAME->getEnv("HOME")+"/.config/sqlitestudio";
#endif
}

QString ConfigImpl::getPortableConfigPath()
{
    QStringList paths = QStringList({"./sqlitestudio-cfg", qApp->applicationDirPath() + "/sqlitestudio-cfg"});
    QSet<QString> pathSet;
    QDir dir;
    for (const QString& path : paths)
    {
        dir = QDir(path);
        pathSet << dir.absolutePath();
    }

    QString potentialPath;
    QFileInfo file;
    for (const QString& path : pathSet)
    {
        dir = QDir(path);
        file = QFileInfo(dir.absolutePath());
        if (!file.exists())
        {
            if (potentialPath.isNull())
                potentialPath = dir.absolutePath();

            continue;
        }

        if (!file.isDir() || !file.isReadable() || !file.isWritable())
            continue;

        foreach (file, dir.entryInfoList())
        {
            if (!file.isReadable() || !file.isWritable())
                continue;
        }

        return dir.absolutePath();
    }

    return potentialPath;
}

void ConfigImpl::initTables()
{
    SqlQueryPtr results = db->exec("SELECT lower(name) AS name FROM sqlite_master WHERE type = 'table'");
    QList<QString> tables = results->columnAsList<QString>(0);

    if (!tables.contains("version"))
    {
        QString table;
        foreach (table, tables)
            db->exec("DROP TABLE "+table);

        tables.clear();
        db->exec("CREATE TABLE version (version NUMERIC)");
        db->exec("INSERT INTO version VALUES ("+QString::number(SQLITESTUDIO_CONFIG_VERSION)+")");
    }

    if (!tables.contains("settings"))
        db->exec("CREATE TABLE settings ([group] TEXT, [key] TEXT, value, PRIMARY KEY([group], [key]))");

    if (!tables.contains("sqleditor_history"))
        db->exec("CREATE TABLE sqleditor_history (id INTEGER PRIMARY KEY, dbname TEXT, date INTEGER, time_spent INTEGER, rows INTEGER, sql TEXT)");

    if (!tables.contains("dblist"))
        db->exec("CREATE TABLE dblist (name TEXT PRIMARY KEY, path TEXT UNIQUE, options TEXT)");

    if (!tables.contains("groups"))
        db->exec("CREATE TABLE groups (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, parent INTEGER REFERENCES groups(id), "
                 "[order] INTEGER, open INTEGER DEFAULT 0, dbname TEXT UNIQUE REFERENCES dblist(name) ON UPDATE CASCADE ON DELETE CASCADE, "
                 "UNIQUE(name, parent))");

    if (!tables.contains("ddl_history"))
        db->exec("CREATE TABLE ddl_history (id INTEGER PRIMARY KEY AUTOINCREMENT, dbname TEXT, file TEXT, timestamp INTEGER, "
                 "queries TEXT)");

    if (!tables.contains("cli_history"))
        db->exec("CREATE TABLE cli_history (id INTEGER PRIMARY KEY AUTOINCREMENT, text TEXT)");

    if (!tables.contains("reports_history"))
        db->exec("CREATE TABLE reports_history (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp INTEGER, feature_request BOOLEAN, title TEXT, url TEXT)");
}

void ConfigImpl::initDbFile()
{
    // Determinate global config location and portable one
    QString globalPath = getConfigPath();
    QString portablePath = getPortableConfigPath();

    QList<QPair<QString,bool>> paths;
    if (!globalPath.isNull() && !portablePath.isNull())
    {
        if (QFileInfo(portablePath).exists())
        {
            paths << QPair<QString,bool>(portablePath+"/"+DB_FILE_NAME, false);
            paths << QPair<QString,bool>(globalPath+"/"+DB_FILE_NAME, true);
        }
        else
        {
            paths << QPair<QString,bool>(globalPath+"/"+DB_FILE_NAME, true);
            paths << QPair<QString,bool>(portablePath+"/"+DB_FILE_NAME, false);
        }
    }
    else if (!globalPath.isNull())
    {
        paths << QPair<QString,bool>(globalPath+"/"+DB_FILE_NAME, true);
    }
    else if (!portablePath.isNull())
    {
        paths << QPair<QString,bool>(portablePath+"/"+DB_FILE_NAME, false);
    }

    // A fallback to in-memory db
    paths << QPair<QString,bool>(":memory:", false);

    // Go through all candidates and pick one
    QDir dir;
    for (const QPair<QString,bool>& path : paths)
    {
        dir = QDir(path.first);
        if (path.first != ":memory:")
            dir.cdUp();

        if (tryInitDbFile(path))
        {
            configDir = dir.absolutePath();
            break;
        }
    }

    // We ended up with in-memory one? That's not good.
    if (configDir == ":memory:")
    {
        paths.removeLast();
        QStringList pathStrings;
        for (const QPair<QString,bool>& path : paths)
            pathStrings << path.first;

        notifyError(QObject::tr("Could not initialize configuration file. Any configuration changes and queries history will be lost after application restart."
                       " Tried to initialize the file at following localizations: %1.").arg(pathStrings.join(", ")));
    }

    qDebug() << "Using configuration directory:" << configDir;
    db->exec("PRAGMA foreign_keys = 1;");
}

bool ConfigImpl::tryInitDbFile(const QPair<QString, bool> &dbPath)
{
    // Create global config directory if not existing
    if (dbPath.second && !dbPath.first.isNull())
    {
        QDir dir(dbPath.first.mid(0, dbPath.first.length() - DB_FILE_NAME.length() - 1));
        if (!dir.exists())
            QDir::root().mkpath(dir.absolutePath());
    }

    db = new DbSqlite3("SQLiteStudio settings", dbPath.first, {{DB_PURE_INIT, true}});
    if (!db->open())
    {
        safe_delete(db);
        return false;
    }

    SqlQueryPtr results = db->exec("SELECT * FROM sqlite_master");
    if (results->isError())
    {
        safe_delete(db);
        return false;
    }

    return true;
}

QVariant ConfigImpl::deserializeValue(const QVariant &value)
{
    if (!value.isValid())
        return QVariant();

    QByteArray bytes = value.toByteArray();
    if (bytes.isNull())
        return QVariant();

    QVariant deserializedValue;
    QDataStream stream(bytes);
    stream >> deserializedValue;
    return deserializedValue;
}
