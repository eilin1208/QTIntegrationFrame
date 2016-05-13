#include "IntegrationFrame.h"
#include "common/unused.h"
#include "services/pluginmanager.h"
#include "plugins/plugin.h"
#include "plugins/plugintype.h"
#include "impl/pluginmanagerimpl.h"


#include <QProcessEnvironment>
#include <QThreadPool>
#include <QCoreApplication>

DEFINE_SINGLETON(IntegrationFrame)

static const int IntegrationFrameVersion = 30008;

IntegrationFrame::IntegrationFrame(QObject *parent) : QObject(parent)
{
    if (qApp) // qApp is null in unit tests
        connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

void IntegrationFrame::init(const QStringList& cmdListArguments, bool guiAvailable)
{
    env = new QProcessEnvironment(QProcessEnvironment::systemEnvironment());
    this->guiAvailable = guiAvailable;

    QThreadPool::globalInstance()->setMaxThreadCount(10);

    Q_INIT_RESOURCE(coreIntegrationFrame);

    pluginManager = new PluginManagerImpl();

}

void IntegrationFrame::initPlugins()
{
    pluginManager->init();
}

void IntegrationFrame::cleanUp()
{
    //disconnect(pluginManager, SIGNAL(aboutToUnload(Plugin*,PluginType*)), this, SLOT(pluginToBeUnloaded(Plugin*,PluginType*)));
    //disconnect(pluginManager, SIGNAL(unloaded(QString,PluginType*)), this, SLOT(pluginUnloaded(QString,PluginType*)));
    if (!immediateQuit)
    {
        if (pluginManager)
            pluginManager->deinit();

        safe_delete(pluginManager); // PluginManager before DbManager, so Db objects are deleted while DbManager still exists
    }
}

void IntegrationFrame::pluginLoaded(Plugin* plugin, PluginType* pluginType)
{
    UNUSED(plugin);
    UNUSED(pluginType);
    //if (pluginType->isForPluginType<CodeFormatterPlugin>()) // TODO move this to slot of CodeFormatter
        //updateCodeFormatter();
}

void IntegrationFrame::pluginToBeUnloaded(Plugin* plugin, PluginType* pluginType)
{
    UNUSED(plugin);
    UNUSED(pluginType);
}

void IntegrationFrame::pluginUnloaded(const QString& pluginName, PluginType* pluginType)
{
    UNUSED(pluginName);
    UNUSED(pluginType);
    //if (pluginType->isForPluginType<CodeFormatterPlugin>()) // TODO move this to slot of CodeFormatter
    //    updateCodeFormatter();
}

QString IntegrationFrame::getEnv(const QString &name, const QString &defaultValue)
{
    return env->value(name, defaultValue);
}

PluginManager* IntegrationFrame::getPluginManager() const
{
    return pluginManager;
}

void IntegrationFrame::setPluginManager(PluginManager* value)
{
    safe_delete(pluginManager);
    pluginManager = value;
}

