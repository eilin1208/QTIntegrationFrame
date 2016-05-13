#include "IntegrationFrame.h"

#include <QProcessEnvironment>
#include <QThreadPool>

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
}

void IntegrationFrame::initPlugins()
{

}

void IntegrationFrame::cleanUp()
{

}

QString IntegrationFrame::getEnv(const QString &name, const QString &defaultValue)
{
    return env->value(name, defaultValue);
}
