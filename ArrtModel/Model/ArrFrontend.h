#pragma once
#include <Model/IncludesAzureRemoteRendering.h>
#include <Model/Settings/AccountConnectionStatus.h>
#include <QObject>
#include <condition_variable>

namespace Microsoft::Azure::RemoteRendering::Internal
{
    class RemoteRenderingClient;
}; // namespace Microsoft::Azure::RemoteRendering::Internal

// wrapper on a RR::AzureFrontend, the class responsible for starting sessions and controlling conversion

class ArrFrontend : public QObject
{
    Q_OBJECT
public:
    ArrFrontend(QObject* parent);
    ~ArrFrontend();

    // return the underlying RR::AzureFrontEnd instance
    RR::ApiHandle<RR::RemoteRenderingClient>& getClient() { return m_rrClient; }

    void connectAccount(const char* accountID, const char* accountKey, const char* accountDomain, const char* region);
    void reconnectAccount();

    AccountConnectionStatus getStatus() const { return m_status; }

Q_SIGNALS:
    void onStatusChanged();

private:
    std::string m_region;
    std::string m_accountId;
    std::string m_accountKey;
    std::string m_accountDomain;

    RR::ApiHandle<RR::RemoteRenderingClient> m_rrClient;
    RR::Status m_sessionPropertiesAsyncStatus = RR::Status::OK;

    AccountConnectionStatus m_status = AccountConnectionStatus::NotAuthenticated;

    std::condition_variable m_condVar;
    std::mutex m_mutex;
#ifndef NDEBUG
    std::atomic<int> m_reentryCnt{0};
#endif

    void connect();
    void setStatus(AccountConnectionStatus status);
};
