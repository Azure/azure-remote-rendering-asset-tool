#include <QTimer>
#include <ViewModel/Session/RunningSessionModel.h>
#include <ctime>

using namespace std::chrono_literals;

namespace
{
    int minutes(const RR::ARRTimeSpan& span)
    {
        return span.hour * 60 + span.minute;
    }
} // namespace

RunningSessionModel::RunningSessionModel(ArrSessionManager* sessionManager, Configuration* configuration, QObject* parent)
    : SessionModel(sessionManager, configuration, parent)
{
}

RunningSessionModel::Size RunningSessionModel::getSize() const
{
    if (isRunningImpl(true))
    {
        return (Size)m_sessionManager->getSessionDescriptor().m_size;
    }
    else
    {
        return {};
    }
}

RunningSessionModel::Time RunningSessionModel::getLeaseTime() const
{
    if (isRunningImpl(true))
    {
        return minutes(m_sessionManager->getSessionDescriptor().m_maxLeaseTime);
    }
    else
    {
        return {};
    }
}

RunningSessionModel::Time RunningSessionModel::getRemainingTime() const
{
    return m_sessionManager->getRemainingMinutes();
}

bool RunningSessionModel::isAutomaticallyExtended() const
{
    uint ignoredParameter;
    bool automaticallyExtended;
    m_sessionManager->getExtensionTime(ignoredParameter, automaticallyExtended);
    return automaticallyExtended;
}

void RunningSessionModel::setAutomaticallyExtended(bool autoExtension)
{
    uint minutes;
    bool ignoredParameter;
    m_sessionManager->getExtensionTime(minutes, ignoredParameter);
    m_sessionManager->setExtensionTime(minutes, autoExtension);
}

void RunningSessionModel::extend()
{
    m_sessionManager->extendMaxSessionTime();
}

RunningSessionModel::Time RunningSessionModel::getExtensionTime() const
{
    uint minutes;
    bool ignoredParameter;
    m_sessionManager->getExtensionTime(minutes, ignoredParameter);
    return minutes;
}

void RunningSessionModel::setExtensionTime(Time extensionTime)
{
    uint ignoredParameter;
    bool automaticallyExtended;
    m_sessionManager->getExtensionTime(ignoredParameter, automaticallyExtended);
    m_sessionManager->setExtensionTime(extensionTime.m_totalMinutes, automaticallyExtended);
}

// return the id of the running version
QString RunningSessionModel::getHostName() const
{
    if (isRunning())
    {
        return QString::fromStdString(m_sessionManager->getSessionDescriptor().m_hostName);
    }
    else
    {
        return {};
    }
}

QString RunningSessionModel::getLastMessage() const
{
    if (isRunning())
    {
        return QString::fromStdString(m_sessionManager->getSessionStatus().m_sessionMessage);
    }
    else
    {
        return {};
    }
}

void RunningSessionModel::startInspector() const
{
    m_sessionManager->startInspector();
}

bool RunningSessionModel::canInspectSession() const
{
    auto status = m_sessionManager->getSessionStatus().m_status;
    return status == SessionStatus::Status::ReadyConnected || status == SessionStatus::Status::ReadyConnecting || status == SessionStatus::Status::ReadyNotConnected;
}
