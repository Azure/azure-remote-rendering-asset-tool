#include <QDebug>
#include <ViewModel/Session/SessionModel.h>

SessionModel::SessionModel(ArrSessionManager* sessionManager, Configuration* configuration, QObject* parent)
    : QObject(parent)
    , m_configuration(configuration)
    , m_sessionManager(sessionManager)
{
    QObject::connect(m_sessionManager, &ArrSessionManager::changed, this, [this]() { Q_EMIT changed(); });
    Q_EMIT changed();
}

bool SessionModel::isRunning() const
{
    return isRunningImpl(false);
}

bool SessionModel::isRunningImpl(bool sessionHasToBeKnown) const
{
    auto status = m_sessionManager->getSessionStatus();
    if (!status.isRunning())
    {
        return false;
    }
    // if the flag sessionHasToBeKnown is true it means that a "starting" state has to return false, because the session can't be queried
    return !(sessionHasToBeKnown && status.m_status == SessionStatus::Status::StartRequested);
}

SessionModel::Status SessionModel::getStatus() const
{
    return m_sessionManager->getSessionStatus().m_status;
}

bool SessionModel::isLoading() const
{
    switch (m_sessionManager->getSessionStatus().m_status)
    {
        case SessionStatus::Status::NotActive:
        case SessionStatus::Status::Stopped:
        case SessionStatus::Status::Expired:
        case SessionStatus::Status::Error:
        case SessionStatus::Status::ReadyNotConnected:
        case SessionStatus::Status::ReadyConnected:
            return false;
        case SessionStatus::Status::StartRequested:
        case SessionStatus::Status::Starting:
        case SessionStatus::Status::ReadyConnecting:
        case SessionStatus::Status::StopRequested:
            return true;
        default:
            return false;
    }
}

bool SessionModel::stop()
{
    if (isRunning())
    {
        const bool succeeded = m_sessionManager->stopSession();
        changed();
        if (!succeeded)
        {
            qWarning() << tr("Session didn't successfully stop");
        }
        return succeeded;
    }
    else
    {
        // already stopped. Cannot stop again
        return false;
    }
}
