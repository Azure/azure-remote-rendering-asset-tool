#include <View/ArrtStyle.h>
#include <View/Session/SessionInfoButton.h>
#include <ViewModel/ModelEditor/ModelEditorModel.h>
#include <ViewModel/ModelsPage/ModelsPageModel.h>
#include <ViewModel/NotificationButtonModel.h>
#include <ViewModel/NotificationButtonModelImplementation.h>
#include <ViewModel/Render/RenderPageModel.h>
#include <ViewModel/Session/SessionPanelModel.h>
#include <Widgets/TimeValidator.h>

namespace
{
    NotificationButtonModel::Notification::Type statusToNotificationType(SessionStatus::Status status)
    {
        typedef NotificationButtonModel::Notification::Type NotificationType;
        switch (status)
        {
            default:
            case SessionStatus::Status::NotActive:
                return NotificationType::Session_NotActive;
            case SessionStatus::Status::Stopped:
                return NotificationType::Session_Stopped;
            case SessionStatus::Status::Expired:
                return NotificationType::Session_Expired;
            case SessionStatus::Status::Error:
                return NotificationType::Session_Error;
            case SessionStatus::Status::StartRequested:
                return NotificationType::Session_StartRequested;
            case SessionStatus::Status::Starting:
                return NotificationType::Session_Starting;
            case SessionStatus::Status::ReadyNotConnected:
                return NotificationType::Session_ReadyNotConnected;
            case SessionStatus::Status::ReadyConnecting:
                return NotificationType::Session_ReadyConnecting;
            case SessionStatus::Status::ReadyConnected:
                return NotificationType::Session_ReadyConnected;
            case SessionStatus::Status::StopRequested:
                return NotificationType::Session_StopRequested;
        }
    }
} // namespace

RenderPageModel::RenderPageModel(AzureStorageManager* storageManager, ArrSessionManager* sessionManager, Configuration* configuration, QObject* parent)
    : QObject(parent)
    , m_sessionManager(sessionManager)
    , m_sessionPanelModel(new SessionPanelModel(sessionManager, configuration, this))
    , m_modelEditor(new ModelEditorModel(sessionManager, this))
    , m_modelsPage(new ModelsPageModel(storageManager, sessionManager, configuration, this))
    , m_buttonModel(new NotificationButtonModelImplementation(this))
{
    QObject::connect(m_modelsPage, &ModelsPageModel::modelLoaded, this, [this](bool loaded) {
        m_isModelLoaded = loaded;
        updateFocusedPage();
    });

    QObject::connect(sessionManager, &ArrSessionManager::changed, this, [this]() {
        updateFocusedPage();
    });

    QObject::connect(m_modelsPage, &ModelsPageModel::loadingStatusChanged, this, [this]() {
        if (m_modelsPage->getCurrentLoadingStatus() == BlobsListModel::LoadingStatus::LOADING)
        {
            m_buttonModel->setProgress(true, m_modelsPage->getCurrentLoadingProgress() * 100.0f);
        }
        else
        {
            m_buttonModel->setProgress(false);
        }
    });

    auto updateSessionStatus = [this, sessionManager]() {
        auto status = sessionManager->getSessionStatus().m_status;
        if (status != m_status)
        {
            m_status = status;
            m_buttonModel->setNotifications({{statusToNotificationType(m_status)}});
            m_buttonModel->setStatusString(ArrtStyle::formatParameterList({tr("Current session status"), tr("Remaining time")})
                                               .arg(SessionInfoButton::getStringFromStatus(status))
                                               .arg(TimeValidator::minutesToString(sessionManager->getRemainingMinutes())));
        }
    };
    connect(sessionManager, &ArrSessionManager::changed, this, updateSessionStatus);
    updateSessionStatus();
}

NotificationButtonModel* RenderPageModel::getNotificationButtonModel() const
{
    return m_buttonModel;
}

RenderPageModel::FocusedPage RenderPageModel::getFocusedPage() const
{
    //check the status
    if (!m_isModelLoaded || m_sessionManager->getSessionStatus().m_status != SessionStatus::Status::ReadyConnected)
    {
        return FocusedPage::MODEL_SELECTION;
    }
    else
    {
        return FocusedPage::MODEL_EDITING;
    }
}

void RenderPageModel::updateFocusedPage()
{
    auto newFocusedPage = getFocusedPage();
    if (newFocusedPage != m_focusedPage)
    {
        m_focusedPage = newFocusedPage;
        Q_EMIT focusedPageChanged();
    }
}

SessionPanelModel* RenderPageModel::getSessionPanelModel() const
{
    return m_sessionPanelModel;
}

ModelEditorModel* RenderPageModel::getModelEditorModel() const
{
    return m_modelEditor;
}

ModelsPageModel* RenderPageModel::getModelsPageModel() const
{
    return m_modelsPage;
}
