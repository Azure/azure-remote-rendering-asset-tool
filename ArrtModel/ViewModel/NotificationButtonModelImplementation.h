#pragma once
#include <ViewModel/NotificationButtonModel.h>

// implementation of MainButtonModel, only accessible from the view model, not exposed to the view
class NotificationButtonModelImplementation : public NotificationButtonModel
{
    Q_OBJECT
public:
    using NotificationButtonModel::NotificationButtonModel;

    //implementation of MainButtonModel
    virtual bool isInProgress() const override;
    virtual int getProgress() const override;
    virtual const std::vector<Notification>& getNotifications() const override;
    virtual void setVisualized(bool visualized) override;
    virtual QString getStatusString() const override;

    void setProgress(bool inProgress, int progressPercentage = s_infiniteProgress);
    void setNotifications(std::vector<Notification> notifications);
    bool isVisualized() const;
    void setStatusString(QString statusString);

Q_SIGNALS:
    void onVisualizedChanged(bool visualized);

private:
    bool m_inProgress = false;
    int m_progressPercentage = s_infiniteProgress;
    std::vector<Notification> m_notifications;
    bool m_visualized = false;
    QString m_statusString;
};
