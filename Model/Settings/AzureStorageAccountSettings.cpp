#include <Model/Settings/AzureStorageAccountSettings.h>
#include <QMetaEnum>
#include <QMetaObject>
#include <Utils/JsonUtils.h>

AzureStorageAccountSettings::AzureStorageAccountSettings(QObject* parent)
    : QObject(parent)
{
}

void AzureStorageAccountSettings::loadFromJson(const QJsonObject& azureStorageAccountSettings)
{
    m_name = azureStorageAccountSettings[QLatin1String("name")].toString();
    m_key = azureStorageAccountSettings[QLatin1String("key")].toString();
    m_blobEndpoint = azureStorageAccountSettings[QLatin1String("blobendpoint")].toString();
}

QJsonObject AzureStorageAccountSettings::saveToJson() const
{
    QJsonObject azureStorageAccountSettings;
    azureStorageAccountSettings[QLatin1String("name")] = m_name;
    azureStorageAccountSettings[QLatin1String("key")] = m_key;
    azureStorageAccountSettings[QLatin1String("blobendpoint")] = m_blobEndpoint;
    return azureStorageAccountSettings;
}
