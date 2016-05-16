#include "mainwindow.h"
#include "common/unused.h"
#include "UnityFrame.h"
#include "mdiarea.h"
#include "mdichild.h"
#include "iconmanager.h"
#include "uiconfig.h"
#include "common/extaction.h"
#include "services/notifymanager.h"
#include "services/pluginmanager.h"
//#include "uidebug.h"

#include <QDebug>
#include <QMdiSubWindow>
#include <QUiLoader>
#include <QStyleFactory>
#include <QInputDialog>
#include <QProgressBar>
#include <QLabel>

CFG_KEYS_DEFINE(MainWindow)
MainWindow* MainWindow::instance = nullptr;

MainWindow::MainWindow() :
    QMainWindow(),
    ui(new Ui::MainWindow)
{
    init();
}

MainWindow::~MainWindow()
{
}

void MainWindow::init()
{
    ui->setupUi(this);
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
#ifdef Q_OS_WIN
    setWindowIcon(ICONS.SQLITESTUDIO_APP.toQIcon().pixmap(256, 256));
#else
    setWindowIcon(ICONS.SQLITESTUDIO_APP);
#endif

    setWindowTitle(QString("UnityFrame (%1)").arg(UNITYFRAME->getVersionString()));

    initActions();

}

MainWindow *MainWindow::getInstance()
{
    if (!instance)
        instance = new MainWindow();

    return instance;
}

void MainWindow::createActions()
{

}

void MainWindow::setupDefShortcuts()
{
    BIND_SHORTCUTS(MainWindow, Action);
}


void MainWindow::cleanUp()
{
    if (UNITYFRAME->getImmediateQuit())
        return;

    for (MdiWindow* win : getMdiArea()->getWindows())
        delete win;

    delete ui;
}

MdiArea *MainWindow::getMdiArea() const
{
    return dynamic_cast<MdiArea*>(ui->mdiArea);
}

void MainWindow::restoreSession()
{

}

void MainWindow::pushClosedWindowSessionValue(const QVariant &value)
{
    closedWindowSessionValues.enqueue(value);

    if (closedWindowSessionValues.size() > closedWindowsStackSize)
        closedWindowSessionValues.dequeue();
}

void MainWindow::restoreLastClosedWindow()
{
    if (closedWindowSessionValues.size() == 0)
        return;

    QMdiSubWindow* activeWin = ui->mdiArea->activeSubWindow();
    bool maximizedMode = activeWin && activeWin->isMaximized();

    QVariant winSession = closedWindowSessionValues.takeLast();
    if (maximizedMode)
    {
        QHash<QString, QVariant> winSessionHash = winSession.toHash();
        winSessionHash.remove("geometry");
        winSession = winSessionHash;
    }

    restoreWindowSession(winSession);
}

bool MainWindow::hasClosedWindowToRestore() const
{
    return closedWindowSessionValues.size() > 0;
}


bool MainWindow::isClosingApp() const
{
    return closingApp;
}

QToolBar* MainWindow::getToolBar(int toolbar) const
{

    return nullptr;
}

void MainWindow::updateWindowActions()
{
    bool hasActiveTask = ui->mdiArea->activeSubWindow();

}

void MainWindow::restoreWindowSessions(const QList<QVariant>& windowSessions)
{
    if (windowSessions.size() == 0)
        return;

    foreach (const QVariant& winSession, windowSessions)
        restoreWindowSession(winSession);
}

MdiWindow* MainWindow::restoreWindowSession(const QVariant &windowSessions)
{
    QHash<QString, QVariant> winSessionHash = windowSessions.toHash();
    if (!winSessionHash.contains("class"))
        return nullptr;

    // Find out the type of stored session
    QByteArray classBytes = winSessionHash["class"].toString().toLatin1();
    char* className = classBytes.data();
    int type = QMetaType::type(className);
    if (type == QMetaType::UnknownType)
    {
        qWarning() << "Could not restore window session, because type" << className
                   << "is not known to Qt meta subsystem.";
        return nullptr;
    }

    // Try to instantiate the object
    void* object = QMetaType::create(type);
    if (!object)
    {
        qWarning() << "Could not restore window session, because type" << className
                   << "could not be instantiated.";
        return nullptr;
    }

    // Switch to session aware window, so we can use its session aware interface.
    MdiChild* mdiChild = reinterpret_cast<MdiChild*>(object);
    if (mdiChild->isInvalid())
    {
        delete mdiChild;
        return nullptr;
    }

    // Add the window to MDI area and restore its session
    MdiWindow* window = ui->mdiArea->addSubWindow(mdiChild);
    if (!window->restoreSession(winSessionHash))
    {
        window->setCloseWithoutSessionSaving(true);
        delete window;
    }

    return window;
}
