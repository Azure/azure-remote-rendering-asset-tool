#include <ViewModel/NotificationButtonModelImplementation.h>


bool NotificationButtonModelImplementation::isInProgress() const
{
    return m_inProgress;
}

int NotificationButtonModelImplementation::getProgress() const
{
    return m_progressPercentage;
}

const std::vector<NotificationButtonModel::Notification>& NotificationButtonModelImplementation::getNotifications() const
{
    return m_notifications;
}

void NotificationButtonModelImplementation::setVisualized(bool visualized)
{
    if (m_visualized != visualized)
    {
        m_visualized = visualized;
        Q_EMIT onVisualizedChanged(m_visualized);
    }
}

QString NotificationButtonModelImplementation::getStatusString() const
{
    return m_statusString;
}

void NotificationButtonModelImplementation::setProgress(bool inProgress, int progressPercentage)
{
    m_inProgress = inProgress;
    m_progressPercentage = progressPercentage;
    Q_EMIT progressChanged();
}

void NotificationButtonModelImplementation::setNotifications(std::vector<Notification> notifications)
{
    m_notifications = std::move(notifications);
    Q_EMIT notificationsChanged();
}

bool NotificationButtonModelImplementation::isVisualized() const
{
    return m_visualized;
}

void NotificationButtonModelImplementation::setStatusString(QString statusString)
{
    m_statusString = std::move(statusString);
    Q_EMIT statusStringChanged();
}
