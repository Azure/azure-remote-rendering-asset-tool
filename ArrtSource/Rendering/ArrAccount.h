#pragma once

#include <QObject>
#include <Rendering/IncludeAzureRemoteRendering.h>
#include <Utils/ConnectionStatus.h>
#include <condition_variable>

namespace RR = Microsoft::Azure::RemoteRendering;

namespace Microsoft::Azure::RemoteRendering::Internal
{
    class RemoteRenderingClient;
}; // namespace Microsoft::Azure::RemoteRendering::Internal

struct ArrRegionInfo
{
    QString m_name;
    QString m_url;
};

struct ArrAccountDomainInfo
{
    QString m_name;
    QString m_url;
};

class ArrAccount : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void ConnectionStatusChanged();

public:
    ArrAccount();
    ~ArrAccount();

    bool LoadSettings();
    void SaveSettings() const;
    void SetSettings(const QString& accountId, const QString& accountKey, const QString& accountDomain, const QString& region);

    void ConnectToArrAccount();
    void DisconnectFromArrAccount();

    QString GetAccountId() const { return m_accountId; }
    QString GetAccountKey() const { return m_accountKey; }
    QString GetAccountDomain() const { return m_accountDomain; }
    QString GetRegion() const { return m_region; }

    AccountConnectionStatus GetConnectionStatus() const { return m_connectionStatus; }
    RR::ApiHandle<RR::RemoteRenderingClient>& GetClient() { return m_rrClient; }

    void GetAvailableRegions(std::vector<ArrRegionInfo>& regions);
    void GetAvailableAccountDomains(std::vector<ArrAccountDomainInfo>& domains);

private:
    void SetConnectionStatus(AccountConnectionStatus newStatus);
    void GetCurrentRenderingSessionsResult(RR::ApiHandle<RR::RemoteRenderingClient> client, RR::Status status, RR::ApiHandle<RR::RenderingSessionPropertiesArrayResult> result);

    QString m_region;
    QString m_accountId;
    QString m_accountKey;
    QString m_accountDomain;

    RR::ApiHandle<RR::RemoteRenderingClient> m_rrClient;
    RR::Status m_querySessionsStatus = RR::Status::OK;

    AccountConnectionStatus m_connectionStatus = AccountConnectionStatus::NotAuthenticated;

    std::condition_variable m_condVar;
    std::mutex m_mutex;

    std::vector<ArrRegionInfo> m_customRegions;
    std::vector<ArrAccountDomainInfo> m_customAccountDomains;

    RR::event_token m_messageLoggedToken;
};