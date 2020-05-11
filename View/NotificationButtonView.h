#pragma once
#include <Widgets/MainToolbarButton.h>
class NotificationButtonModel;

// main application button which gets the information from a MainButtonModel to show notifications
class NotificationButtonView : public MainToolbarButton
{
    Q_OBJECT
public:
    NotificationButtonView(const QString& name, NotificationButtonModel* model, QWidget* parent = {});

    virtual void setToolTip(const QString& title, const QString& details) override;

private:
    NotificationButtonModel* const m_model;
    QString m_tooltip;

    void updateToolTip();
    void updateNotifications();
    void updateProgress();
};
