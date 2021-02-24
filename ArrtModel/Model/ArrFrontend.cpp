#include <Model/ArrFrontend.h>
#include <Model/Log/LogHelpers.h>
#include <QApplication>
#include <QPointer>

ArrFrontend::ArrFrontend(QObject* parent)
    : QObject(parent)
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

void ArrFrontend::connectAccount(const char* accountID, const char* accountKey, const char* accountDomain, const char* region)
{
    if (m_region == region &&
        m_accountId == accountID &&
        m_accountKey == accountKey &&
        m_accountDomain == accountDomain)
    {
        return;
    }

    m_region = region;
    m_accountId = accountID;
    m_accountKey = accountKey;
    m_accountDomain = accountDomain;

    connect();
}

void ArrFrontend::reconnectAccount()
{
    connect();
}

void ArrFrontend::connect()
{
    assert(m_reentryCnt.fetch_add(1) == 0 && m_sessionPropertiesAsync != RR::Status::InProgress);

    if (m_rrClient)
    {
        setStatus(AccountConnectionStatus::NotAuthenticated);
        m_rrClient = nullptr;
    }

    if (!m_region.empty() && !m_accountId.empty() && !m_accountKey.empty() && !m_accountDomain.empty())
    {
        setStatus(AccountConnectionStatus::CheckingCredentials);

        RR::SessionConfiguration fi{};
        fi.AccountDomain = m_accountDomain;
        fi.RemoteRenderingDomain = m_region;
        fi.AccountId = m_accountId;
        fi.AccountKey = m_accountKey;

        auto client = RR::ApiHandle(RR::RemoteRenderingClient(fi));
        client->MessageLogged(&qArrSdkMessage);
        client->SetLogLevel(RR::LogLevel::Debug);
        QPointer<ArrFrontend> thisPtr = this;
        m_sessionPropertiesAsync = RR::Status::InProgress;
        client->GetCurrentRenderingSessionsAsync([thisPtr, client](RR::Status status, RR::ApiHandle<RR::RenderingSessionPropertiesArrayResult> result) {
            {
                std::unique_lock<std::mutex> lk(thisPtr->m_mutex);
                thisPtr->m_sessionPropertiesAsync = status;
                thisPtr->m_condVar.notify_one();
            }
            QMetaObject::invokeMethod(QApplication::instance(),
                                      [thisPtr, client, status, result]() {
                                          if (thisPtr != nullptr)
                                          {
                                              if (status == RR::Status::OK && result->GetContext().Result == RR::Result::Success)
                                              {
                                                  thisPtr->m_rrClient = client;
                                                  thisPtr->setStatus(AccountConnectionStatus::Authenticated);
                                              }
                                              else
                                              {
                                                  thisPtr->setStatus(AccountConnectionStatus::InvalidCredentials);
                                                  qWarning(LoggingCategory::configuration) << tr("Failed to get rendering sessions. Possibly invalid account data.");
                                              }
                                          }
                                      });
        });
    }

#ifndef NDEBUG
    m_reentryCnt.fetch_sub(1);
#endif
}

ArrFrontend::~ArrFrontend()
{
    // Wait for current rendering sessions query to complete.
    std::unique_lock<std::mutex> lk(m_mutex);
    if (m_sessionPropertiesAsync == RR::Status::InProgress)
    {
        m_condVar.wait(lk);
    }

    m_rrClient = nullptr;
    RR::ShutdownRemoteRendering();
}

void ArrFrontend::setStatus(AccountConnectionStatus status)
{
    if (m_status != status)
    {
        m_status = status;
        Q_EMIT onStatusChanged();
    }
}
