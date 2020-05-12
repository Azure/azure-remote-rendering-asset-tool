#pragma once
#include <Widgets/FlatButton.h>
class QTimer;

// button used in the main toolbar. It can display a small notification on its top right corner with icon and text.

class MainToolbarButton : public FlatButton
{
public:
    MainToolbarButton(const QString& name, QWidget* parent = {});

    // start an animation on the progress, used when there is no clear progress percentage
    void startProgressAnimation();

    // set the progress percentage
    void setProgress(int progressPercentage);

    // stop the animation and removes the progess
    void stopProgress();

protected:
    struct Notification
    {
        QIcon m_icon;
        QColor m_color;
        QString m_string;
    };

    void setNotifications(std::vector<Notification> notifications);
    virtual void paintEvent(QPaintEvent* e) override;

private:
    QTimer* const m_loadingAnimationTimer;
    int m_loadingAnimationTime = 0;
    int m_progress;
    std::vector<Notification> m_notifications;
};
