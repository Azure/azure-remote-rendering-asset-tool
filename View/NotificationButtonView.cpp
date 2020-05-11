#include <View/ArrtStyle.h>
#include <View/NotificationButtonView.h>
#include <ViewModel/NotificationButtonModel.h>
#include <Widgets/FormControl.h>

namespace
{
    struct ColorIcon
    {
        ColorIcon()
        {
        }

        ColorIcon(QColor color, QIcon icon)
            : m_color(std::move(color))
            , m_icon(std::move(icon))
        {
        }

        ColorIcon(QColor color)
            : m_color(std::move(color))
        {
        }

        QColor m_color;
        QIcon m_icon;
    };

    ColorIcon getIconColorFromNotificationType(NotificationButtonModel::Notification::Type notificationType)
    {
        typedef NotificationButtonModel::Notification::Type NotificationType;
        switch (notificationType)
        {
            default:
            case NotificationType::Undefined:
                return {};
            case NotificationType::Debug:
                return {ArrtStyle::s_debugColor, ArrtStyle::s_debugIcon};
            case NotificationType::Warning:
                return {ArrtStyle::s_warningColor, ArrtStyle::s_warningIcon};
            case NotificationType::Error:
                return {ArrtStyle::s_errorColor, ArrtStyle::s_criticalIcon};
            case NotificationType::Info:
                return {ArrtStyle::s_infoColor, ArrtStyle::s_infoIcon};
            case NotificationType::Running:
                return {ArrtStyle::s_runningColor, ArrtStyle::s_conversion_runningIcon};
            case NotificationType::Completed:
                return {ArrtStyle::s_successColor, ArrtStyle::s_conversion_succeededIcon};
            case NotificationType::Failed:
                return {ArrtStyle::s_failureColor, ArrtStyle::s_conversion_failedIcon};
            case NotificationType::Session_NotActive:
                return {Qt::transparent, ArrtStyle::s_session_not_activeIcon};
            case NotificationType::Session_Stopped:
                return {Qt::transparent, ArrtStyle::s_session_stoppedIcon};
            case NotificationType::Session_Expired:
                return {Qt::transparent, ArrtStyle::s_session_expiredIcon};
            case NotificationType::Session_Error:
                return {Qt::transparent, ArrtStyle::s_session_errorIcon};
            case NotificationType::Session_StartRequested:
                return {Qt::transparent, ArrtStyle::s_session_startingIcon};
            case NotificationType::Session_Starting:
                return {Qt::transparent, ArrtStyle::s_session_startingIcon};
            case NotificationType::Session_ReadyNotConnected:
                return {Qt::transparent, ArrtStyle::s_session_readyIcon};
            case NotificationType::Session_ReadyConnecting:
                return {Qt::transparent, ArrtStyle::s_session_connectingIcon};
            case NotificationType::Session_ReadyConnected:
                return {Qt::transparent, ArrtStyle::s_session_connectedIcon};
            case NotificationType::Session_StopRequested:
                return {Qt::transparent, ArrtStyle::s_session_stop_requestedIcon};
        }
    }
} // namespace

NotificationButtonView::NotificationButtonView(const QString& name, NotificationButtonModel* model, QWidget* parent)
    : MainToolbarButton(name, parent)
    , m_model(model)
{
    updateToolTip();
    updateNotifications();
    updateProgress();

    connect(m_model, &NotificationButtonModel::statusStringChanged, this, [this]() {
        updateToolTip();
    });

    connect(m_model, &NotificationButtonModel::notificationsChanged, this, [this]() {
        updateNotifications();
    });

    connect(m_model, &NotificationButtonModel::progressChanged, this, [this]() {
        updateProgress();
    });

    auto onToggled = [this](bool checked) {
        m_model->setVisualized(checked);
    };
    connect(this, &FlatButton::toggled, this, onToggled);
    onToggled(isChecked());
}

void NotificationButtonView::setToolTip(const QString& title, const QString& details)
{
    m_tooltip = ArrtStyle::formatToolTip(title, details);
    updateToolTip();
}

void NotificationButtonView::updateToolTip()
{
    QToolButton::setToolTip(m_tooltip + m_model->getStatusString());
}

void NotificationButtonView::updateNotifications()
{
    std::vector<Notification> notifications;
    notifications.reserve(m_model->getNotifications().size());
    for (const auto& n : m_model->getNotifications())
    {
        auto iconColor = getIconColorFromNotificationType(n.m_type);
        QColor c = iconColor.m_color;
        c.setAlpha(c.alpha() * 2 / 3);
        c = c.darker();

        notifications.push_back({iconColor.m_icon, c, n.m_count ? QString::number(n.m_count.value()) : QString()});
    }

    setNotifications(std::move(notifications));
}

void NotificationButtonView::updateProgress()
{
    if (m_model->isInProgress())
    {
        const auto progress = m_model->getProgress();
        if (progress == NotificationButtonModel::s_infiniteProgress)
        {
            startProgressAnimation();
        }
        else
        {
            setProgress(progress);
        }
    }
    else
    {
        stopProgress();
    }
}
