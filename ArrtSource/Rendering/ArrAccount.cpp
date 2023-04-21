#include <ArrtVersion.h>
#include <QApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QSettings>
#include <QStandardPaths>
#include <Rendering/ArrAccount.h>
#include <Utils/Logging.h>

ArrAccount::ArrAccount()
{
    RR::RemoteRenderingInitialization ci{};
    ci.ConnectionType = RR::ConnectionType::General;
    ci.GraphicsApi = RR::GraphicsApiType::SimD3D11;
    ci.Right = RR::Axis::X;
    ci.Up = RR::Axis::Y;
    ci.Forward = RR::Axis::Z;
    ci.UnitsPerMeter = 1.0F;
    ci.ToolId = std::string("ARRT." ARRT_VERSION);
    RR::StartupRemoteRendering(ci);
}

ArrAccount::~ArrAccount()
{
    DisconnectFromArrAccount();

    RR::ShutdownRemoteRendering();
}

bool ArrAccount::LoadSettings()
{
    QSettings s;
    s.beginGroup("ArrAccount");
    m_accountId = s.value("AccountId").toString();
    m_accountKey = s.value("AccountKey").toString();
    m_accountDomain = s.value("AccountDomain").toString();
    m_region = s.value("Region").toString();
    s.endGroup();

    QDir appDataRootDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    appDataRootDir.mkpath(QString("."));
    QString configFile = appDataRootDir.absolutePath() + QDir::separator() + QString("ARR-config.json");

    QFile file(configFile);

    if (file.open(QIODevice::ReadOnly))
    {
        const QByteArray fileContent = file.readAll();
        QJsonDocument jsonDoc(QJsonDocument::fromJson(fileContent));

        const QJsonObject& rootObj = jsonDoc.object();

        {
            QJsonArray regionsArray = rootObj["CustomRegions"].toArray();

            for (const auto& region : regionsArray)
            {
                QJsonObject obj = region.toObject();
                m_customRegions.push_back({obj["name"].toString(), obj["url"].toString()});
            }
        }

        {
            QJsonArray accountDomainsArray = rootObj["CustomAccountDomains"].toArray();

            for (const auto& domain : accountDomainsArray)
            {
                QJsonObject obj = domain.toObject();
                m_customAccountDomains.push_back({obj["name"].toString(), obj["url"].toString()});
            }
        }
    }

    SanitizeSettings(m_accountId, m_accountKey, m_accountDomain, m_region);

    SaveSettings();

    return !m_accountId.isEmpty() && !m_accountKey.isEmpty();
}

void ArrAccount::SaveSettings() const
{
    QSettings s;
    s.beginGroup("ArrAccount");
    s.setValue("AccountId", m_accountId);
    s.setValue("AccountKey", m_accountKey);
    s.setValue("AccountDomain", m_accountDomain);
    s.setValue("Region", m_region);
    s.endGroup();

    QDir appDataRootDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    appDataRootDir.mkpath(QString("."));
    QString configFile = appDataRootDir.absolutePath() + QDir::separator() + QString("ARR-config.json");

    QFile file(configFile);
    if (file.open(QIODevice::WriteOnly))
    {
        QJsonObject arrObj;
        {
            {
                QJsonArray regionsArray;
                for (auto& region : m_customRegions)
                {
                    QJsonObject obj;
                    obj["name"] = region.m_name;
                    obj["url"] = region.m_url;
                    regionsArray.append(obj);
                }
                arrObj["CustomRegions"] = regionsArray;
            }

            {
                QJsonArray accountDomainsArray;
                for (auto& domain : m_customAccountDomains)
                {
                    QJsonObject obj;
                    obj["name"] = domain.m_name;
                    obj["url"] = domain.m_url;
                    accountDomainsArray.append(obj);
                }
                arrObj["CustomAccountDomains"] = accountDomainsArray;
            }
        }

        QJsonDocument saveDoc(arrObj);
        file.write(saveDoc.toJson());
    }
}

void ArrAccount::SanitizeSettings(QString& accountId, QString& accountKey, QString& accountDomain, QString& region)
{
    accountId = accountId.trimmed();
    accountKey = accountKey.trimmed();
    accountDomain = accountDomain.trimmed();
    region = region.trimmed();

    while (accountDomain.endsWith("/"))
        accountDomain.chop(1);
}

void ArrAccount::SetSettings(QString accountId, QString accountKey, QString accountDomain, QString region)
{
    SanitizeSettings(accountId, accountKey, accountDomain, region);

    if (m_accountId == accountId && m_accountKey == accountKey && m_accountDomain == accountDomain && m_region == region)
        return;

    m_accountId = accountId;
    m_accountKey = accountKey;
    m_accountDomain = accountDomain;
    m_region = region;

    SaveSettings();

    DisconnectFromArrAccount();
}

void ArrAccount::SetConnectionStatus(ArrConnectionStatus newStatus)
{
    if (m_connectionStatus != newStatus)
    {
        m_connectionStatus = newStatus;
        Q_EMIT ConnectionStatusChanged();
    }
}

void ArrAccount::ConnectToArrAccount()
{
    if (m_connectionStatus == ArrConnectionStatus::Authenticated)
        return;

    DisconnectFromArrAccount();

    if (m_region.isEmpty() || m_accountId.isEmpty() || m_accountKey.isEmpty() || m_accountDomain.isEmpty())
        return;

    SetConnectionStatus(ArrConnectionStatus::CheckingCredentials);

    RR::SessionConfiguration fi{};
    fi.AccountDomain = m_accountDomain.toStdString();
    fi.RemoteRenderingDomain = m_region.toStdString();
    fi.AccountId = m_accountId.toStdString();
    fi.AccountKey = m_accountKey.toStdString();

    auto client = RR::ApiHandle(RR::RemoteRenderingClient(fi));
    m_messageLoggedToken = client->MessageLogged(&ForwardArrLogMsgToQt).value();
    client->SetLogLevel(RR::LogLevel::Information);

    m_querySessionsStatus = RR::Status::InProgress;

    auto resultCallback = [this, client](RR::Status status, RR::ApiHandle<RR::RenderingSessionPropertiesArrayResult> result)
    {
        QMetaObject::invokeMethod(QApplication::instance(),
                                  [this, client, status, result]()
                                  {
                                      GetCurrentRenderingSessionsResult(client, status, result);
                                  });
    };

    // we use this call to check whether connecting to the ARR account works properly
    // if this call fails, most likely the used credentials are incorrect
    client->GetCurrentRenderingSessionsAsync(resultCallback);
}

void ArrAccount::GetCurrentRenderingSessionsResult(RR::ApiHandle<RR::RemoteRenderingClient> client, RR::Status status, RR::ApiHandle<RR::RenderingSessionPropertiesArrayResult> result)
{
    RR::Result errorCode = RR::StatusToResult(status);

    if (status == RR::Status::OK)
    {
        errorCode = result->GetErrorCode();
    }

    if (errorCode == RR::Result::Success)
    {
        m_rrClient = client;

        // this could be used to display all currently running sessions somewhere
        std::vector<Microsoft::Azure::RemoteRendering::RenderingSessionProperties> sessions;
        result->GetSessionProperties(sessions);

        SetConnectionStatus(ArrConnectionStatus::Authenticated);
    }
    else
    {
        SetConnectionStatus(ArrConnectionStatus::InvalidCredentials);

        if (errorCode == RR::Result::DomainUnreachable)
        {
            qCritical(LoggingCategory::RenderingSession) << "'" << errorCode << "' - this typically happens when a firewall or router blocks UDP traffic.";
            qCritical(LoggingCategory::RenderingSession) << "Visit https://learn.microsoft.com/azure/remote-rendering/resources/troubleshoot for help.";
        }
        else
        {
            qCritical(LoggingCategory::RenderingSession) << "Failed to get rendering sessions, account credentials might be incorrect: " << errorCode;
        }
    }

    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_querySessionsStatus = status;
        m_condVar.notify_one();
    }
}


void ArrAccount::DisconnectFromArrAccount()
{
    if (m_rrClient == nullptr)
        return;

    // Wait for current rendering sessions query to complete.
    std::unique_lock<std::mutex> lk(m_mutex);
    if (m_querySessionsStatus == RR::Status::InProgress)
    {
        m_condVar.wait(lk);
    }

    if (m_messageLoggedToken)
    {
        // disconnect the message event callback
        m_rrClient->MessageLogged(m_messageLoggedToken);
        m_messageLoggedToken.invalidate();
    }

    SetConnectionStatus(ArrConnectionStatus::NotAuthenticated);
    m_rrClient = nullptr;
}

void ArrAccount::GetAvailableRegions(std::vector<ArrRegionInfo>& regions)
{
    regions.clear();
    regions.push_back({"Australia East", "australiaeast.mixedreality.azure.com"});
    regions.push_back({"East US", "eastus.mixedreality.azure.com"});
    regions.push_back({"East US 2", "eastus2.mixedreality.azure.com"});
    regions.push_back({"Japan East", "japaneast.mixedreality.azure.com"});
    regions.push_back({"North Europe", "northeurope.mixedreality.azure.com"});
    regions.push_back({"South Central US", "southcentralus.mixedreality.azure.com"});
    regions.push_back({"Southeast Asia", "southeastasia.mixedreality.azure.com"});
    regions.push_back({"UK South", "uksouth.mixedreality.azure.com"});
    regions.push_back({"West Europe", "westeurope.mixedreality.azure.com"});
    regions.push_back({"West US 2", "westus2.mixedreality.azure.com"});

    for (const auto& custom : m_customRegions)
    {
        regions.push_back(custom);
    }

    std::sort(regions.begin(), regions.end(), [](const ArrRegionInfo& lhs, const ArrRegionInfo& rhs)
              { return lhs.m_name < rhs.m_name; });
}

void ArrAccount::GetAvailableAccountDomains(std::vector<ArrAccountDomainInfo>& domains)
{
    domains.clear();

    domains.push_back({"Australia East", "australiaeast.mixedreality.azure.com"});
    domains.push_back({"East US", "eastus.mixedreality.azure.com"});
    domains.push_back({"East US 2", "mixedreality.azure.com"});
    domains.push_back({"Japan East", "japaneast.mixedreality.azure.com"});
    domains.push_back({"North Europe", "northeurope.mixedreality.azure.com"});
    domains.push_back({"South Central US", "southcentralus.mixedreality.azure.com"});
    domains.push_back({"Southeast Asia", "southeastasia.mixedreality.azure.com"});
    domains.push_back({"UK South", "uksouth.mixedreality.azure.com"});
    domains.push_back({"West Europe", "westeurope.mixedreality.azure.com"});
    domains.push_back({"West US 2", "westus2.mixedreality.azure.com"});

    for (const auto& custom : m_customAccountDomains)
    {
        domains.push_back(custom);
    }

    std::sort(domains.begin(), domains.end(), [](const ArrAccountDomainInfo& lhs, const ArrAccountDomainInfo& rhs)
              { return lhs.m_name < rhs.m_name; });
}
