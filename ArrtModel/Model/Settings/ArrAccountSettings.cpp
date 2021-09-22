#include <Model/Settings/ArrAccountSettings.h>

#include <Model/Log/LogHelpers.h>
#include <QMetaEnum>
#include <QMetaObject>
#include <Utils/JsonUtils.h>
#include <Utils/StringEncrypter.h>

static const std::map<QString, ArrAccountSettings::Region> defaultAvailableRegionsMap = {
    {"australiaeast.mixedreality.azure.com", {"Australia East", "australiaeast.mixedreality.azure.com"}},
    {"eastus.mixedreality.azure.com", {"East US", "eastus.mixedreality.azure.com"}},
    {"eastus2.mixedreality.azure.com", {"East US 2", "eastus2.mixedreality.azure.com"}},
    {"japaneast.mixedreality.azure.com", {"Japan East", "japaneast.mixedreality.azure.com"}},
    {"northeurope.mixedreality.azure.com", {"North Europe", "northeurope.mixedreality.azure.com"}},
    {"southcentralus.mixedreality.azure.com", {"South Central US", "southcentralus.mixedreality.azure.com"}},
    {"southeastasia.mixedreality.azure.com", {"Southeast Asia", "southeastasia.mixedreality.azure.com"}},
    {"uksouth.mixedreality.azure.com", {"UK South", "uksouth.mixedreality.azure.com"}},
    {"westeurope.mixedreality.azure.com", {"West Europe", "westeurope.mixedreality.azure.com"}},
    {"westus2.mixedreality.azure.com", {"West US 2", "westus2.mixedreality.azure.com"}}};

static const std::map<QString, ArrAccountSettings::AccountDomain> defaultSupportedAccountDomainsMap = {
    {"australiaeast.mixedreality.azure.com", {"Australia East", "australiaeast.mixedreality.azure.com"}},
    {"eastus.mixedreality.azure.com", {"East US", "eastus.mixedreality.azure.com"}},
    {"mixedreality.azure.com", {"East US 2", "mixedreality.azure.com"}},
    {"japaneast.mixedreality.azure.com", {"Japan East", "japaneast.mixedreality.azure.com"}},
    {"northeurope.mixedreality.azure.com", {"North Europe", "northeurope.mixedreality.azure.com"}},
    {"southcentralus.mixedreality.azure.com", {"South Central US", "southcentralus.mixedreality.azure.com"}},
    {"southeastasia.mixedreality.azure.com", {"Southeast Asia", "southeastasia.mixedreality.azure.com"}},
    {"uksouth.mixedreality.azure.com", {"UK South", "uksouth.mixedreality.azure.com"}},
    {"westeurope.mixedreality.azure.com", {"West Europe", "westeurope.mixedreality.azure.com"}},
    {"westus2.mixedreality.azure.com", {"West US 2", "westus2.mixedreality.azure.com"}}};

ArrAccountSettings::ArrAccountSettings(QObject* parent)
    : QObject(parent)
{
    // initialize the regions
    m_availableRegionsMap = defaultAvailableRegionsMap;

    // supported account domains
    m_supportedAccountDomainsMap = defaultSupportedAccountDomainsMap;
}

void ArrAccountSettings::loadFromJson(const QJsonObject& arrAccountConfig)
{
    m_id = arrAccountConfig[QLatin1String("id")].toString();
    m_key = arrAccountConfig[QLatin1String("key")].toString();
    m_accountDomain = arrAccountConfig[QLatin1String("accountDomain")].toString();

    auto oldFormatRegion = arrAccountConfig[QLatin1String("region")].toString();
    if (!oldFormatRegion.isEmpty())
    {
        m_region = oldFormatRegion + ".mixedreality.azure.com";
    }
    else
    {
        m_region = arrAccountConfig[QLatin1String("regionurl")].toString();
    }


    // Load additional regions, if any, from the configuration file.
    QJsonArray regionsArray = arrAccountConfig[QLatin1String("availableregions")].toArray();
    if (!regionsArray.isEmpty())
    {
        for (auto e : regionsArray)
        {
            QJsonObject regionObj = e.toObject();
            ArrAccountSettings::Region region({regionObj[QLatin1String("name")].toString(), regionObj[QLatin1String("url")].toString()});
            m_availableRegionsMap[region.m_domainUrl] = region;
        }
    }

    // Load additional supported account domains, if any, from the configuration file.
    QJsonArray supportedAccountDomainsArray = arrAccountConfig[QLatin1String("supportedAccountDomains")].toArray();
    if (!supportedAccountDomainsArray.isEmpty())
    {
        for (auto e : supportedAccountDomainsArray)
        {
            QJsonObject accountDomainObj = e.toObject();
            ArrAccountSettings::AccountDomain accountDomain({accountDomainObj[QLatin1String("name")].toString(), accountDomainObj[QLatin1String("accountDomain")].toString()});
            m_supportedAccountDomainsMap[accountDomain.m_accountDomain] = accountDomain;
        }
    }
}


QJsonObject ArrAccountSettings::saveToJson() const
{
    QJsonObject arrAccountConfig;
    arrAccountConfig[QLatin1String("id")] = m_id;
    arrAccountConfig[QLatin1String("key")] = m_key;
    arrAccountConfig[QLatin1String("accountDomain")] = m_accountDomain;
    arrAccountConfig[QLatin1String("regionurl")] = m_region;

    // Write additional regions, if any, to the configuration file.
    QJsonArray regionsArray;
    for (auto&& region : m_availableRegionsMap)
    {
        if (defaultAvailableRegionsMap.find(region.first) == defaultAvailableRegionsMap.end())
        {
            QJsonObject regionObj;
            regionObj[QLatin1String("name")] = region.second.m_label;
            regionObj[QLatin1String("url")] = region.second.m_domainUrl;
            regionsArray.append(regionObj);
        }
    }
    arrAccountConfig[QLatin1String("availableregions")] = regionsArray;

    // Write additional supported account domains, if any, to the configuration file.
    QJsonArray supportedAccountDomainsArray;
    for (auto&& accountDomain : m_supportedAccountDomainsMap)
    {
        if (defaultSupportedAccountDomainsMap.find(accountDomain.first) == defaultSupportedAccountDomainsMap.end())
        {
            QJsonObject accountDomainObj;
            accountDomainObj[QLatin1String("name")] = accountDomain.second.m_label;
            accountDomainObj[QLatin1String("accountDomain")] = accountDomain.second.m_accountDomain;
            supportedAccountDomainsArray.append(accountDomainObj);
        }
    }
    arrAccountConfig[QLatin1String("supportedAccountDomains")] = supportedAccountDomainsArray;

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

std::vector<ArrAccountSettings::Region> ArrAccountSettings::getAvailableRegions() const
{
    std::vector<Region> regions;
    regions.reserve(m_availableRegionsMap.size());
    for (const auto& e : m_availableRegionsMap)
    {
        regions.emplace_back(e.second);
    }
    return regions;
}

std::vector<ArrAccountSettings::AccountDomain> ArrAccountSettings::getSupportedAccountDomains() const
{
    std::vector<AccountDomain> accountDomains;
    accountDomains.reserve(m_supportedAccountDomainsMap.size());
    for (auto e : m_supportedAccountDomainsMap)
    {
        accountDomains.emplace_back(e.second);
    }
    return accountDomains;
}
