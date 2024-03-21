#pragma once

#include <QObject>
#include <Rendering/IncludeAzureRemoteRendering.h>
#include <condition_variable>

namespace RR = Microsoft::Azure::RemoteRendering;

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

enum class ArrConnectionStatus
{
    Authenticated,
    NotAuthenticated,
    CheckingCredentials,
    InvalidCredentials
};

/// Manages general interactions with the ARR account
///
/// Does not handle session creation etc, that's done by ArrSession.
class ArrAccount : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void ConnectionStatusChanged();

public:
    ArrAccount();
    virtual ~ArrAccount();

    RR::ApiHandle<RR::RemoteRenderingClient>& GetClient() { return m_rrClient; }

    bool LoadSettings();
    void SaveSettings() const;
    bool SetSettings(QString accountId, QString accountKey, QString accountDomain, QString region);

    QString GetAccountId() const { return m_accountId; }
    QString GetAccountKey() const { return m_accountKey; }
    QString GetAccountDomain() const { return m_accountDomain; }
    QString GetRegion() const { return m_region; }

    virtual void ConnectToArrAccount();
    virtual void DisconnectFromArrAccount();

    ArrConnectionStatus GetConnectionStatus() const { return m_connectionStatus; }

    void GetAvailableRegions(std::vector<ArrRegionInfo>& regions);
    void GetAvailableAccountDomains(std::vector<ArrAccountDomainInfo>& domains);

    void BlockChanges(bool block);

protected:
    static void SanitizeSettings(QString& accountId, QString& accountKey, QString& accountDomain, QString& region);
    void SetConnectionStatus(ArrConnectionStatus newStatus);
    void GetCurrentRenderingSessionsResult(RR::ApiHandle<RR::RemoteRenderingClient> client, RR::Status status, RR::ApiHandle<RR::RenderingSessionPropertiesArrayResult> result);

    QString m_region;
    QString m_accountId;
    QString m_accountKey;
    QString m_accountDomain;

    RR::ApiHandle<RR::RemoteRenderingClient> m_rrClient;
    RR::Status m_querySessionsStatus = RR::Status::OK;

    ArrConnectionStatus m_connectionStatus = ArrConnectionStatus::NotAuthenticated;

    std::condition_variable m_condVar;
    std::mutex m_mutex;

    std::vector<ArrRegionInfo> m_customRegions;
    std::vector<ArrAccountDomainInfo> m_customAccountDomains;

    RR::event_token m_messageLoggedToken;
    bool m_blockChanges = false;
};

/// Mock implementation of ArrAccount to run ARRT UI without account credentials.
class ArrAccountMock : public ArrAccount
{
public:
    void ConnectToArrAccount() override;
    void DisconnectFromArrAccount() override;
};