#ifndef INTEGRATION_H
#define INTEGRATION_H

#include <QObject>
#include "coreUnityFrame_global.h"
#include "common/global.h"
#include "services/config.h"

class QProcessEnvironment;
class QThreadPool;
class PluginManager;
class Plugin;
class PluginType;
class Config;
class DbManager;

class API_EXPORT CUnityFrame : public QObject
{
    Q_OBJECT

    DECLARE_SINGLETON(CUnityFrame)
public:
    explicit CUnityFrame(QObject *parent = 0);

    bool isGuiAvailable() const;

    Config* getConfig() const;
    void setConfig(Config* value);

    int getVersion() const;
    QString getVersionString() const;

    /**
     * @brief Initializes SQLiteStudio object.
     * @param cmdListArguments Command line arguments.
     * @param pluginLoadingHandler Factory for producing plugin loader.
     *
     * Initialization process involves creating of all internal objects (managers, etc.)
     * and reading necessary configuration. It also interpreted command line arguments
     * and applies them.
     *
     * The plugin loader factory (handler) is used to solve issue with GUI symbols visibility. while loading code being placed in the core shared library.
     * It should be null when starting SQLiteStudio in CLI mode and not null when starting GUI client. See PluginLoadingHandler for more details on that.
     *
     * See parseCmdLineArgs() for details on supported options.
     */
    void init(const QStringList& cmdListArguments, bool guiAvailable);

    void initPlugins();

    /**
     * @brief Gets environment variable value.
     * @param name Name of the environment variable.
     * @param defaultValue Default value to be returned if the environment variable is not defined.
     * @return Either value of the environment variable - if defined - or the value passed as defaultValue.
     *
     * This method provides cross-platform way to get environment variable value.
     * Internally it uses QProcessEnvironment, but while it's expensive to initialize it every time you access environment,
     * it keeps the single instance of that object and lets you query variables by name.
     */
    QString getEnv(const QString& name, const QString& defaultValue = QString());

    PluginManager* getPluginManager() const;
    void setPluginManager(PluginManager* value);

    QStringList getInitialTranslationFiles() const;
    void setInitialTranslationFiles(const QStringList& value);

    DbManager* getDbManager() const;
    void setDbManager(DbManager* value);

    bool getImmediateQuit() const;
    void setImmediateQuit(bool value);

    QString getCurrentLang() const;

private slots:
    void pluginLoaded(Plugin* plugin,PluginType* pluginType);
    void pluginToBeUnloaded(Plugin* plugin,PluginType* pluginType);
    void pluginUnloaded(const QString& pluginName,PluginType* pluginType);
    /**
     * @brief Cleans up all internal objects.
     *
     * Deletes all internal objects. It's called from destructor.
     */
    void cleanUp();

private:
    /**
     * @brief The application environment.
     *
     * This variable represents environment of the application.
     * It provides access to environment variables.
     */
    QProcessEnvironment* env = nullptr;

    bool guiAvailable = false;
    bool immediateQuit = false;
    Config* config = nullptr;
    PluginManager* pluginManager = nullptr;
    DbManager* dbManager = nullptr;

    QString currentLang;

    QStringList initialTranslationFiles;
signals:

public slots:
};

#define UNITYFRAME CUnityFrame::getInstance()

#endif // INTEGRATION_H
