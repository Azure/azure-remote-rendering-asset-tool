#pragma once
#include <Model/ArrSessionManager.h>
#include <QObject>
#include <ViewModel/Session/SessionModel.h>

class ArrSessionManager;
class QTimer;
class Configuration;

// model class for the the panel to start a session SessionCreationView

class SessionCreationModel : public SessionModel
{
    Q_OBJECT
public:
    SessionCreationModel(ArrSessionManager* sessionManager, Configuration* configuration, QObject* parent);
    virtual ~SessionCreationModel();

    Size getSize() const;
    void setSize(Size size);

    // get the initial lease time
    Time getLeaseTime() const;
    // setting the initial lease time
    void setLeaseTime(const Time& leaseTime);

    // if this is true elapsedTime > maxTime-(extensionTime/4), then extend() is called
    bool isAutomaticallyExtended() const;
    void setAutomaticallyExtended(bool autoExtension);

    // the amount of time the lease time will be extended when calling "extend" (explicitly or automatically)
    Time getExtensionTime() const;
    void setExtensionTime(Time extensionTime);

    // try to start a session
    bool start();

    bool isEnabled() const;

Q_SIGNALS:
    void onEnabledChanged();

private:
    Time m_leaseTime;
    Size m_size;
    int m_extensionMinutes;
    bool m_extendAutomatically;
};
