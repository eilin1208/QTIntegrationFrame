#include "mainwindow.h"
#include "UnityFrame.h"

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QApplication>
#include <QSplashScreen>
#include <QThread>
#include <QPluginLoader>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>


bool listPlugins = false;

QString uiHandleCmdLineArgs()
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("GUI interface to SQLiteStudio, a SQLite manager."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption debugOption({"d", "debug"}, QObject::tr("Enables debug messages in console (accessible with F12)."));
    QCommandLineOption debugStdOutOption("debug-stdout", QObject::tr("Redirects debug messages into standard output (forces debug mode)."));
    QCommandLineOption debugFileOption("debug-file", QObject::tr("Redirects debug messages into given file (forces debug mode)."), QObject::tr("log file"));
    QCommandLineOption lemonDebugOption("debug-lemon", QObject::tr("Enables Lemon parser debug messages for SQL code assistant."));
    QCommandLineOption sqlDebugOption("debug-sql", QObject::tr("Enables debugging of every single SQL query being sent to any database."));
    QCommandLineOption sqlDebugDbNameOption("debug-sql-db", QObject::tr("Limits SQL query messages to only the given <database>."), QObject::tr("database"));
    QCommandLineOption listPluginsOption("list-plugins", QObject::tr("Lists plugins installed in the SQLiteStudio and quits."));
    parser.addOption(debugOption);
    parser.addOption(debugStdOutOption);
    parser.addOption(debugFileOption);
    parser.addOption(lemonDebugOption);
    parser.addOption(sqlDebugOption);
    parser.addOption(sqlDebugDbNameOption);
    parser.addOption(listPluginsOption);

    parser.addPositionalArgument(QObject::tr("file"), QObject::tr("Database file to open"));

    parser.process(qApp->arguments());

    bool enableDebug = parser.isSet(debugOption) || parser.isSet(debugStdOutOption) || parser.isSet(sqlDebugOption) || parser.isSet(debugFileOption);
    setUiDebug(enableDebug, !parser.isSet(debugStdOutOption), parser.value(debugFileOption));
    CompletionHelper::enableLemonDebug = parser.isSet(lemonDebugOption);
    setSqlLoggingEnabled(parser.isSet(sqlDebugOption));
    if (parser.isSet(sqlDebugDbNameOption))
        setSqlLoggingFilter(parser.value(sqlDebugDbNameOption));

    if (parser.isSet(listPluginsOption))
        listPlugins = true;

    QStringList args = parser.positionalArguments();
    if (args.size() > 0)
        return args[0];

    return QString::null;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    qInstallMessageHandler(uiMessageHandler);
    QString dbToOpen = uiHandleCmdLineArgs();

    UNITYFRAME->setInitialTranslationFiles({"coreUnityFrame", "guiUnityFrame", "UnityFrame"});
    UNITYFRAME->init(a.arguments(), true);

    MainWindow::getInstance();

    IconManager::getInstance()->rescanResources();

    // Shortcuts titles needs to be retranslated, because their titles were set initially in global scope,
    // while translation files were not loaded yet. Now they are.
    ExtActionContainer::refreshShortcutTranslations();

    MainWindow::getInstance()->restoreSession();
    MainWindow::getInstance()->show();
    if (!dbToOpen.isNull())
        MainWindow::getInstance()->openDb(dbToOpen);

    return a.exec();
}
