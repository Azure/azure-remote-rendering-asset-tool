#pragma once
#include <Model/ArrSessionManager.h>
#include <QObject>
#include <ViewModel/Session/SessionModel.h>

class ArrSessionManager;
class QTimer;

// model class for the the session info panel SessionInfoView

class RunningSessionModel : public SessionModel
{
    Q_OBJECT
public:
    RunningSessionModel(ArrSessionManager* sessionManager, Configuration* configuration, QObject* parent);

    Size getSize() const;

    // get the initial lease time
    Time getLeaseTime() const;

    // return hours and minutes before the expiration time
    Time getRemainingTime() const;

    // if this is true elapsedTime > maxTime-(extensionTime/4), then extend() is called
    bool isAutomaticallyExtended() const;
    void setAutomaticallyExtended(bool autoExtension);

    // extend the lease time by extensionTime
    void extend();

    // the amount of time the lease time will be extended when calling "extend" (explicitly or automatically)
    Time getExtensionTime() const;
    void setExtensionTime(Time extensionTime);

    // return the id of the running version
    QString getHostName() const;

    // return the last message, as passed by the server
    QString getLastMessage() const;

    // start the arrInspector on the running session
    void startInspector() const;

    // return true when the session can be inspected
    bool canInspectSession() const;
};
