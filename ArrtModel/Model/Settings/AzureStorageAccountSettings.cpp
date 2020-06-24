#include <Model/Settings/AzureStorageAccountSettings.h>

#include <Model/Log/LogHelpers.h>
#include <QMetaEnum>
#include <QMetaObject>
#include <Utils/JsonUtils.h>
#include <Utils/StringEncrypter.h>

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

QString AzureStorageAccountSettings::getKey() const
{
    QString key;
    if (StringEncrypter::decrypt(m_key, key))
    {
        return key;
    }
    else
    {
        qWarning(LoggingCategory::configuration) << tr("Error decrypting Azure Storage account key");
        return {};
    }
}

bool AzureStorageAccountSettings::setKey(const QString& key)
{
    if (StringEncrypter::encrypt(key, m_key))
    {
        Q_EMIT changed();
        return true;
    }
    else
    {
        qWarning(LoggingCategory::configuration) << tr("Error encrypting ARR account key");
        return false;
    }
}