#include <Model/AzureStorageManager.h>
#include <Model/Configuration.h>
#include <Model/Settings/AzureStorageAccountSettings.h>
#include <ViewModel/Parameters/TextModel.h>
#include <ViewModel/Settings/StorageAccountSettingsModel.h>
#include <string_view>

StorageAccountSettingsModel::StorageAccountSettingsModel(AzureStorageAccountSettings* azureStorageAccountSettings, AzureStorageManager* storageManager, QObject* parent)
    : SettingsBaseModel(parent)
    , m_azureStorageAccountSettings(azureStorageAccountSettings)
    , m_storageManager(storageManager)
{
    using namespace std::literals;
    m_controls.push_back(new TextModel(tr("Name"), m_azureStorageAccountSettings, "name"sv, true));
    m_controls.push_back(new TextModel(tr("Key"), m_azureStorageAccountSettings, "key"sv, true, true));
    m_controls.push_back(new TextModel(tr("Blob endpoint (e.g. https://[s.a. name].blob.core.windows.net/)"), m_azureStorageAccountSettings, "blobEndpoint"sv, true));

    connect(m_storageManager, &AzureStorageManager::onStatusChanged, this, [this]() {
        Q_EMIT updateUi();
    });

    auto connectAccount = [this]() {
        m_storageManager->connectAccount(
            m_azureStorageAccountSettings->getBlobEndpoint().toStdWString().c_str(),
            m_azureStorageAccountSettings->getName().toStdWString().c_str(),
            m_azureStorageAccountSettings->getKey().toStdWString().c_str());
    };
    connect(m_azureStorageAccountSettings, &AzureStorageAccountSettings::changed, this, connectAccount);
    connectAccount();
}

bool StorageAccountSettingsModel::isEnabled() const
{
    return getStatus() != AccountConnectionStatus::Connecting;
}

AccountConnectionStatus StorageAccountSettingsModel::getStatus() const
{
    return m_storageManager->getStatus();
}

void StorageAccountSettingsModel::reconnectAccount()
{
    m_storageManager->reconnectAccount();
}
