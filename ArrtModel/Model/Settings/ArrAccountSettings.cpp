#include <Model/Settings/ArrAccountSettings.h>

#include <Model/Log/LogHelpers.h>
#include <QMetaEnum>
#include <QMetaObject>
#include <Utils/JsonUtils.h>
#include <Utils/StringEncrypter.h>

ArrAccountSettings::ArrAccountSettings(QObject* parent)
    : QObject(parent)
{
    // initialize the regions
    m_availableRegions = {
        {"Australia East", "australiaeast.mixedreality.azure.com"},
        {"East US", "eastus.mixedreality.azure.com"},
        {"East US 2", "eastus2.mixedreality.azure.com"},
        {"Japan East", "japaneast.mixedreality.azure.com"},
        {"North Europe", "northeurope.mixedreality.azure.com"},
        {"South Central US", "southcentralus.mixedreality.azure.com"},
        {"Southeast Asia", "southeastasia.mixedreality.azure.com"},
        {"UK South", "uksouth.mixedreality.azure.com"},
        {"West Europe", "westeurope.mixedreality.azure.com"},
        {"West US", "westus2.mixedreality.azure.com"}};
}

void ArrAccountSettings::loadFromJson(const QJsonObject& arrAccountConfig)
{
    m_id = arrAccountConfig[QLatin1String("id")].toString();
    m_key = arrAccountConfig[QLatin1String("key")].toString();

    auto oldFormatRegion = arrAccountConfig[QLatin1String("region")].toString();
    if (!oldFormatRegion.isEmpty())
    {
        m_region = oldFormatRegion + ".mixedreality.azure.com";
    }
    else
    {
        m_region = arrAccountConfig[QLatin1String("regionurl")].toString();
    }

    QJsonArray regionsArray = arrAccountConfig[QLatin1String("availableregions")].toArray();
    if (!regionsArray.isEmpty())
    {
        m_availableRegions.clear();
        for (auto e : regionsArray)
        {
            QJsonObject regionObj = e.toObject();
            m_availableRegions.push_back({regionObj[QLatin1String("name")].toString(), regionObj[QLatin1String("url")].toString()});
        }
    }
}

QJsonObject ArrAccountSettings::saveToJson() const
{
    QJsonObject arrAccountConfig;
    arrAccountConfig[QLatin1String("id")] = m_id;
    arrAccountConfig[QLatin1String("key")] = m_key;
    arrAccountConfig[QLatin1String("regionurl")] = m_region;

    QJsonArray regionsArray;
    for (auto&& region : m_availableRegions)
    {
        QJsonObject regionObj;
        regionObj[QLatin1String("name")] = region.m_label;
        regionObj[QLatin1String("url")] = region.m_domainUrl;
        regionsArray.append(regionObj);
    }
    arrAccountConfig[QLatin1String("availableregions")] = regionsArray;

    return arrAccountConfig;
}

QString ArrAccountSettings::getKey() const
{
    QString key;
    if (StringEncrypter::decrypt(m_key, key))
    {
        return key;
    }
    else
    {
        qWarning(LoggingCategory::configuration) << tr("Error decrypting ARR account key");
        return {};
    }
}

bool ArrAccountSettings::setKey(const QString& key)
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
