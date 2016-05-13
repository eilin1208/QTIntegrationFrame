#ifndef INTEGRATION_H
#define INTEGRATION_H

#include <QObject>
#include "coreIntegrationFrame_global.h"
#include "common/global.h"

class QProcessEnvironment;
class QThreadPool;

class API_EXPORT IntegrationFrame : public QObject
{
    Q_OBJECT

    DECLARE_SINGLETON(IntegrationFrame)
public:
    explicit IntegrationFrame(QObject *parent = 0);


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
private slots:
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
signals:

public slots:
};

#define INTEGRATIONFRAME IntegrationFrame::getInstance()

#endif // INTEGRATION_H
