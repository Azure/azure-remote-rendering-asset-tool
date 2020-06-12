#pragma once
#include <Model/ArrSessionManager.h>
#include <QObject>

class ArrSessionManager;
class QTimer;

// base class for the session models

class SessionModel : public QObject
{
    Q_OBJECT
public:
    typedef SessionStatus::Status Status;

    SessionModel(ArrSessionManager* sessionManager, Configuration* configuration, QObject* parent);

    // return true when the session is running or starting was requested
    bool isRunning() const;

    // size of the VM
    using Size = RR::RenderingSessionVmSize;

    // time structure, holding minutes and hours. It represents a duration
    struct Time
    {
        Time(int hours, int minutes)
            : m_totalMinutes(hours * 60 + minutes)
        {
        }
        Time(int minutes = 0)
            : m_totalMinutes(minutes)
        {
        }
        uint32_t m_totalMinutes;
        uint32_t getHours() const { return m_totalMinutes / 60; }
        uint32_t getMinutes() const { return m_totalMinutes % 60; }
    };

    // return the status of the session
    Status getStatus() const;

    // return the loading status (used to display an animation on the button)
    bool isLoading() const;

    // stop the running session
    bool stop();

Q_SIGNALS:
    // called when any change happens (status, lease time, hostname, lastmessage)
    void changed();

protected:
    Configuration* const m_configuration;

    // returns true is the session is running/starting/stopping and if sessionHasToBeKnown==false also when start() was just called
    // successfully but we don't know the session ID yet
    bool isRunningImpl(bool sessionHasToBeKnown) const;

    ArrSessionManager* const m_sessionManager;
};
