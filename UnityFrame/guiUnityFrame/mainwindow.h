#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "common/extactioncontainer.h"
#include "db/db.h"
#include "ui_mainwindow.h"
#include "mdiwindow.h"
#include "common/global.h"
#include "guiUnityFrame_global.h"
#include <QMainWindow>
#include <QHash>
#include <QQueue>

class MdiWindow;
class MdiChild;
class QUiLoader;
class MdiArea;
class QActionGroup;
class Committable;
class QProgressBar;
class QLabel;
class ThemeTuner;

#ifdef Q_OS_MACX
#define PREV_TASK_KEY_SEQ Qt::CTRL + Qt::ALT + Qt::Key_Left
#define NEXT_TASK_KEY_SEQ Qt::CTRL + Qt::ALT + Qt::Key_Right
#else
#define PREV_TASK_KEY_SEQ Qt::CTRL + Qt::Key_PageUp
#define NEXT_TASK_KEY_SEQ Qt::CTRL + Qt::Key_PageDown
#endif

CFG_KEY_LIST(MainWindow, QObject::tr("Main window"),
     CFG_KEY_ENTRY(OPEN_SQL_EDITOR,    Qt::ALT + Qt::Key_E,         QObject::tr("Open SQL editor"))
     CFG_KEY_ENTRY(PREV_TASK,          PREV_TASK_KEY_SEQ,           QObject::tr("Previous window"))
     CFG_KEY_ENTRY(NEXT_TASK,          NEXT_TASK_KEY_SEQ,           QObject::tr("Next window"))
     CFG_KEY_ENTRY(HIDE_STATUS_FIELD,  Qt::Key_Escape,              QObject::tr("Hide status area"))
     CFG_KEY_ENTRY(OPEN_CONFIG,        Qt::Key_F2,                  QObject::tr("Open configuration dialog"))
     CFG_KEY_ENTRY(OPEN_DEBUG_CONSOLE, Qt::Key_F12,                 QObject::tr("Open Debug Console"))
     CFG_KEY_ENTRY(OPEN_CSS_CONSOLE,   Qt::Key_F11,                 QObject::tr("Open CSS Console"))
)


class GUI_API_EXPORT MainWindow : public QMainWindow, public ExtActionContainer
{
    Q_OBJECT
    Q_ENUMS(Action)

public:
    enum Action
    {
        MDI_TILE,
        MDI_CASCADE,
        MDI_TILE_HORIZONTAL,
        MDI_TILE_VERTICAL,
        OPEN_SQL_EDITOR,
        NEXT_TASK,
        PREV_TASK,
        HIDE_STATUS_FIELD,
        OPEN_CONFIG,
        OPEN_DDL_HISTORY,
        OPEN_FUNCTION_EDITOR,
        OPEN_COLLATION_EDITOR,
        EXPORT,
        IMPORT,
        CLOSE_WINDOW,
        CLOSE_ALL_WINDOWS,
        CLOSE_OTHER_WINDOWS,
        RESTORE_WINDOW,
        RENAME_WINDOW,
        OPEN_DEBUG_CONSOLE,
        OPEN_CSS_CONSOLE,
        LICENSES,
        HOMEPAGE,
        FORUM,
        USER_MANUAL,
        SQLITE_DOCS,
        REPORT_BUG,
        FEATURE_REQUEST,
        ABOUT,
        BUG_REPORT_HISTORY,
        CHECK_FOR_UPDATES
    };

public:
    static MainWindow* getInstance();

    MdiArea* getMdiArea() const;

    void restoreSession();
    void pushClosedWindowSessionValue(const QVariant& value);
    bool hasClosedWindowToRestore() const;
    bool isClosingApp() const;
    QToolBar* getToolBar(int toolbar) const;

private:
    MainWindow();
    ~MainWindow();

    void init();
    void createActions();
    void setupDefShortcuts();
    void restoreWindowSessions(const QList<QVariant>& windowSessions);
    MdiWindow *restoreWindowSession(const QVariant& windowSessions);

    template <class T>
    T* openMdiWindow();

    static MainWindow* instance;

    static constexpr int closedWindowsStackSize = 20;
    static_char* openUpdatesUrl = "open_updates://";

public slots:
    void updateWindowActions();

private slots:
    void notifyAboutLanguageChange();
    void cleanUp();

    void restoreLastClosedWindow();

private:
    Ui::MainWindow *ui=nullptr;
    bool closingApp = false;
    QQueue<QVariant> closedWindowSessionValues;
};

template <class T>
T* MainWindow::openMdiWindow()
{
    T* win = nullptr;
    foreach (MdiWindow* mdiWin, ui->mdiArea->getWindows())
    {
        win = dynamic_cast<T*>(mdiWin->getMdiChild());
        if (win)
        {
            ui->mdiArea->setActiveSubWindow(mdiWin);
            return win;
        }
    }

    win = new T(ui->mdiArea);
    if (win->isInvalid())
    {
        delete win;
        return nullptr;
    }

    ui->mdiArea->addSubWindow(win);
    return win;
}

#define MAINWINDOW MainWindow::getInstance()

#endif // MAINWINDOW_H
