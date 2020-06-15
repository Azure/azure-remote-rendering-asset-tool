#pragma once
#include <QObject>
#include <optional>

// model holding the data for a button with progress and notification indicators
class NotificationButtonModel : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    struct Notification
    {
        enum class Type
        {
            Undefined,
            Debug,
            Info,
            Warning,
            Error,
            Running,
            Completed,
            Failed,
            Session_NotActive,
            Session_Stopped,
            Session_Expired,
            Session_Error,
            Session_StartRequested,
            Session_Starting,
            Session_ReadyNotConnected,
            Session_ReadyConnecting,
            Session_ReadyConnected,
            Session_StopRequested
        };

        Notification(Type t, int count)
            : m_type(t)
            , m_count(count)
        {
        }
        Notification(Type t)
            : m_type(t)
        {
        }

        const Type m_type;

        // value to show in the notification. If it's not specified it won't be shown
        const std::optional<int> m_count;
    };


    // return true if the button has to show some progress (the line on top of the button)
    virtual bool isInProgress() const = 0;

    // progress in percentage, or s_infiniteProgress when the progress can't be quantified.
    virtual int getProgress() const = 0;

    // return the notifications to display
    virtual const std::vector<Notification>& getNotifications() const = 0;

    // when the view is visualized, usually any counter that might be displayed is reset
    virtual void setVisualized(bool visualized) = 0;

    // return a description of the status, depending on the notification data. This is usually displayed in the button tooltip coming from the view.
    virtual QString getStatusString() const = 0;

    // when this is returned by getProgress, it means that the indicator will just show a "loading" animation
    static const int s_infiniteProgress = -1;


Q_SIGNALS:
    void progressChanged();
    void notificationsChanged();
    void statusStringChanged();
};
