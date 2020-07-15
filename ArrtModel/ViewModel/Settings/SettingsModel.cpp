#include <ViewModel/Settings/SettingsModel.h>

#include <Model/Configuration.h>
#include <Utils/StyleUtils.h>
#include <ViewModel/NotificationButtonModelImplementation.h>
#include <ViewModel/Settings/ArrAccountSettingsModel.h>
#include <ViewModel/Settings/CameraSettingsModel.h>
#include <ViewModel/Settings/StorageAccountSettingsModel.h>
#include <ViewModel/Settings/VideoSettingsModel.h>

SettingsModel::SettingsModel(Configuration* configuration, ArrFrontend* frontend, AzureStorageManager* storageManager, ArrSessionManager* arrSessionManager, QObject* parent)
    : QObject(parent)
    , m_arrAccountSettingsModel(new ArrAccountSettingsModel(configuration->getArrAccountSettings(), frontend, this))
    , m_storageAccountSettingsModel(new StorageAccountSettingsModel(configuration->getAzureStorageAccountSettings(), storageManager, this))
    , m_videoSettingsModel(new VideoSettingsModel(configuration->getVideoSettings(), arrSessionManager, this))
    , m_cameraSettingsModel(new CameraSettingsModel(configuration->getCameraSettings(), this))
    , m_buttonModel(new NotificationButtonModelImplementation(this))
{
    auto updateLambda = [this]() { updateButton(); };
    connect(m_arrAccountSettingsModel, &ArrAccountSettingsModel::updateUi, this, updateLambda);
    connect(m_storageAccountSettingsModel, &StorageAccountSettingsModel::updateUi, this, updateLambda);
    connect(m_videoSettingsModel, &VideoSettingsModel::updateUi, this, updateLambda);
    updateButton();
}

QString SettingsModel::getVersion() const
{
    return QString(ARRT_VERSION);
}

NotificationButtonModel* SettingsModel::getNotificationButtonModel() const
{
    return m_buttonModel;
}

void SettingsModel::updateButton()
{
    NotificationButtonModel::Notification::Type notificationType = NotificationButtonModel::Notification::Type::Undefined;

    const AccountConnectionStatus saStatus = m_storageAccountSettingsModel->getStatus();
    const AccountConnectionStatus arrStatus = m_arrAccountSettingsModel->getStatus();
    const bool videoFormatSupported = m_videoSettingsModel->isVideoFormatSupported();

    QString statusString = StyleUtils::formatParameterList({tr("ARR connection status"), tr("Storage account status")}).arg(toString(arrStatus)).arg(toString(saStatus));

    if (!videoFormatSupported)
    {
        statusString += tr("<br><b>Unsupported video format.</b>");
    }

    if (saStatus == AccountConnectionStatus::FailedToConnect || arrStatus == AccountConnectionStatus::FailedToConnect || !videoFormatSupported)
    {
        notificationType = NotificationButtonModel::Notification::Type::Error;
    }
    else if (saStatus != AccountConnectionStatus::Connected || arrStatus != AccountConnectionStatus::Connected)
    {
        notificationType = NotificationButtonModel::Notification::Type::Warning;
    }

    m_buttonModel->setProgress(saStatus == AccountConnectionStatus::Connecting || arrStatus == AccountConnectionStatus::Connecting);
    m_buttonModel->setStatusString(statusString);
    if (notificationType != NotificationButtonModel::Notification::Type::Undefined)
    {
        m_buttonModel->setNotifications({{notificationType}});
    }
    else
    {
        m_buttonModel->setNotifications({});
    }
}
