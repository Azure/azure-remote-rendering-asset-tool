#pragma once
#include <Model/ArrSessionManager.h>
#include <QObject>

class ArrSessionManager;
class SessionCreationModel;
class RunningSessionModel;

// model for the session panel view, which shows either the session creation or the running session information

class SessionPanelModel : public QObject
{
    Q_OBJECT

public:
    SessionPanelModel(ArrSessionManager* sessionManager, Configuration* configuration, QObject* parent);

    typedef SessionStatus::Status Status;

    bool isRunning() const { return m_running; }

    Status getStatus() const;
    QString getRemainingTime() const;

    SessionCreationModel* getSessionCreationModel() const { return m_creationModel; }
    RunningSessionModel* getRunningSessionModel() const { return m_runningModel; }

Q_SIGNALS:
    void isRunningChanged();
    void sessionChanged();

private:
    ArrSessionManager* const m_sessionManager;
    SessionCreationModel* const m_creationModel;
    RunningSessionModel* const m_runningModel;

    bool m_running = false;
    int m_remainingMinutes = 0;
    Status m_status = Status::NotActive;
};
