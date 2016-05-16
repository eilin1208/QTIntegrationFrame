#include "UnityFrame.h"
#include "common/unused.h"
#include "services/pluginmanager.h"
#include "plugins/plugin.h"
#include "plugins/plugintype.h"
#include "impl/pluginmanagerimpl.h"
#include "common/utils.h"
#include "common/utils_sql.h"
#include "parser/keywords.h"
#include "parser/lexer.h"
#include "services/notifymanager.h"

#include <QtGlobal>
#include <QProcessEnvironment>
#include <QThreadPool>
#include <QCoreApplication>

DEFINE_SINGLETON(CUnityFrame)

static const int UnityFrameVersion = 10001;

CUnityFrame::CUnityFrame(QObject *parent) : QObject(parent)
{
    if (qApp) // qApp is null in unit tests
        connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

void CUnityFrame::init(const QStringList& cmdListArguments, bool guiAvailable)
{
    env = new QProcessEnvironment(QProcessEnvironment::systemEnvironment());
    this->guiAvailable = guiAvailable;

    QThreadPool::globalInstance()->setMaxThreadCount(10);

    //Q_INIT_RESOURCE(coreUnityFrame);

    CfgLazyInitializer::init();

    initUtils();
    CfgMain::staticInit();
    Db::metaInit();
    initUtilsSql();
    initKeywords();
    Lexer::staticInit();
    //CompletionHelper::init();

    NotifyManager::getInstance();

    config = nullptr;

    pluginManager = new PluginManagerImpl();

    cmdLineArgs = cmdListArguments;
}

bool CUnityFrame::getImmediateQuit() const
{
    return immediateQuit;
}

void CUnityFrame::setImmediateQuit(bool value)
{
    immediateQuit = value;
}

void CUnityFrame::initPlugins()
{
    pluginManager->init();
}

void CUnityFrame::cleanUp()
{
    //disconnect(pluginManager, SIGNAL(aboutToUnload(Plugin*,PluginType*)), this, SLOT(pluginToBeUnloaded(Plugin*,PluginType*)));
    //disconnect(pluginManager, SIGNAL(unloaded(QString,PluginType*)), this, SLOT(pluginUnloaded(QString,PluginType*)));
    if (!immediateQuit)
    {
        if (pluginManager)
            pluginManager->deinit();

        safe_delete(pluginManager); // PluginManager before DbManager, so Db objects are deleted while DbManager still exists
        NotifyManager::destroy();
    }
    //Q_CLEANUP_RESOURCE(coreUnityFrame);
}

void CUnityFrame::pluginLoaded(Plugin* plugin, PluginType* pluginType)
{
    UNUSED(plugin);
    UNUSED(pluginType);
    //if (pluginType->isForPluginType<CodeFormatterPlugin>()) // TODO move this to slot of CodeFormatter
        //updateCodeFormatter();
}

void CUnityFrame::pluginToBeUnloaded(Plugin* plugin, PluginType* pluginType)
{
    UNUSED(plugin);
    UNUSED(pluginType);
}

void CUnityFrame::pluginUnloaded(const QString& pluginName, PluginType* pluginType)
{
    UNUSED(pluginName);
    UNUSED(pluginType);
    //if (pluginType->isForPluginType<CodeFormatterPlugin>()) // TODO move this to slot of CodeFormatter
    //    updateCodeFormatter();
}

QString CUnityFrame::getEnv(const QString &name, const QString &defaultValue)
{
    return env->value(name, defaultValue);
}

PluginManager* CUnityFrame::getPluginManager() const
{
    return pluginManager;
}

void CUnityFrame::setPluginManager(PluginManager* value)
{
    safe_delete(pluginManager);
    pluginManager = value;
}

int CUnityFrame::getVersion() const
{
    return UnityFrameVersion;
}

QString CUnityFrame::getVersionString() const
{
    int ver = getVersion();
    int majorVer = ver / 10000;
    int minorVer = ver % 10000 / 100;
    int patchVer = ver % 100;
    return QString::number(majorVer) + "." + QString::number(minorVer) + "." + QString::number(patchVer);
}

Config* CUnityFrame::getConfig() const
{
    return config;
}

void CUnityFrame::setConfig(Config* value)
{
    safe_delete(config);
    config = value;
}

bool CUnityFrame::isGuiAvailable() const
{
    return guiAvailable;
}

QString CUnityFrame::getCurrentLang() const
{
    return currentLang;
}

QStringList CUnityFrame::getInitialTranslationFiles() const
{
    return initialTranslationFiles;
}

void CUnityFrame::setInitialTranslationFiles(const QStringList& value)
{
    initialTranslationFiles = value;
}

DbManager* CUnityFrame::getDbManager() const
{
    return dbManager;
}

void CUnityFrame::setDbManager(DbManager* value)
{
    safe_delete(dbManager);
    dbManager = value;
}
