#pragma once

#include <QObject>

class Configuration;
class ArrFrontend;
class AzureStorageManager;
class ArrSessionManager;
class ArrAccountSettingsModel;
class StorageAccountSettingsModel;
class VideoSettingsModel;
class CameraSettingsModel;
class NotificationButtonModel;
class NotificationButtonModelImplementation;

// Collection of settings model.

class SettingsModel : public QObject
{
public:
    SettingsModel(Configuration* configuration, ArrFrontend* frontend, AzureStorageManager* storageManager, ArrSessionManager* arrSessionManager, QObject* parent = nullptr);

    NotificationButtonModel* getNotificationButtonModel() const;

    ArrAccountSettingsModel* getArrAccountSettingsModel() const { return m_arrAccountSettingsModel; }
    StorageAccountSettingsModel* getStorageAccountSettingsModel() const { return m_storageAccountSettingsModel; }
    VideoSettingsModel* getVideoSettingsModel() const { return m_videoSettingsModel; }
    CameraSettingsModel* getCameraSettingsModel() const { return m_cameraSettingsModel; }

private:
    ArrAccountSettingsModel* const m_arrAccountSettingsModel;
    StorageAccountSettingsModel* const m_storageAccountSettingsModel;
    VideoSettingsModel* const m_videoSettingsModel;
    CameraSettingsModel* const m_cameraSettingsModel;
    NotificationButtonModelImplementation* const m_buttonModel;

    void updateButton();
};
