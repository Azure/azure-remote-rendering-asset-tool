#include <Model/ArrSessionManager.h>
#include <ViewModel/Session/RunningSessionModel.h>
#include <ViewModel/Session/SessionCreationModel.h>
#include <ViewModel/Session/SessionPanelModel.h>
#include <Utils/TimeValidator.h>

SessionPanelModel::SessionPanelModel(ArrSessionManager* sessionManager, Configuration* configuration, QObject* parent)
    : QObject(parent)
    , m_sessionManager(sessionManager)
    , m_creationModel(new SessionCreationModel(sessionManager, configuration, this))
    , m_runningModel(new RunningSessionModel(sessionManager, configuration, this))
{
    auto updateRunning = [this]() {
        auto sessionStatus = m_sessionManager->getSessionStatus();
        if (sessionStatus.m_status != m_status || m_sessionManager->getRemainingMinutes() != m_remainingMinutes)
        {
            if (sessionStatus.isRunning() != m_running)
            {
                m_running = !m_running;
                Q_EMIT isRunningChanged();
            }
            m_status = m_sessionManager->getSessionStatus().m_status;
            m_remainingMinutes = m_sessionManager->getRemainingMinutes();
            Q_EMIT sessionChanged();
        }
    };
    connect(m_sessionManager, &ArrSessionManager::changed, this, updateRunning);
    updateRunning();
}

SessionPanelModel::Status SessionPanelModel::getStatus() const
{
    return m_sessionManager->getSessionStatus().m_status;
}

QString SessionPanelModel::getRemainingTime() const
{
    return TimeValidator::minutesToString(m_remainingMinutes);
}


QString SessionPanelModel::getStringFromStatus(SessionPanelModel::Status status)
{
    switch (status)
    {
        case SessionPanelModel::Status::NotActive:
            return tr("Not Active");
        case SessionPanelModel::Status::Stopped:
            return tr("Stopped");
        case SessionPanelModel::Status::Expired:
            return tr("Expired");
        case SessionPanelModel::Status::Error:
            return tr("Error");
        case SessionPanelModel::Status::StartRequested:
            return tr("Start Requested");
        case SessionPanelModel::Status::Starting:
            return tr("Starting");
        case SessionPanelModel::Status::ReadyNotConnected:
            return tr("Session Ready");
        case SessionPanelModel::Status::ReadyConnecting:
            return tr("Connecting");
        case SessionPanelModel::Status::ReadyConnected:
            return tr("Connected");
        case SessionPanelModel::Status::StopRequested:
            return tr("Stop Requested");
        default:
            return tr("Not Recognized");
    }
}