#pragma once

#include <Model/Settings/AccountConnectionStatus.h>
#include <ViewModel/Settings/SettingsBaseModel.h>

class AzureStorageAccountSettings;
class AzureStorageManager;

class StorageAccountSettingsModel : public SettingsBaseModel
{
public:
    StorageAccountSettingsModel(AzureStorageAccountSettings* azureStorageAccountSettings, AzureStorageManager* storageManager, QObject* parent);

    bool isEnabled() const override;
    AccountConnectionStatus getStatus() const;

    void reconnectAccount();

private:
    AzureStorageAccountSettings* const m_azureStorageAccountSettings;
    AzureStorageManager* const m_storageManager;
};
