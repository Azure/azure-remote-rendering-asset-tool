#include <Model/ArrFrontend.h>
#include <Model/Log/LogHelpers.h>
#include <QApplication>
#include <QPointer>

ArrFrontend::ArrFrontend(QObject* parent)
    : QObject(parent)
{
    RR::RemoteRenderingInitialization ci;
    memset(&ci, 0, sizeof(RR::RemoteRenderingInitialization));
    ci.connectionType = RR::ConnectionType::General;
    ci.graphicsApi = RR::GraphicsApiType::SimD3D11;
    ci.right = RR::Axis::X;
    ci.up = RR::Axis::Y;
    ci.forward = RR::Axis::Z;
    ci.unitsPerMeter = 1.0F;
    ci.toolId = std::string("ARRT." ARRT_VERSION);
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
    assert(m_reentryCnt.fetch_add(1) == 0 && m_sessionPropertiesAsync == nullptr);

    if (m_rrFrontend)
    {
        setStatus(AccountConnectionStatus::NotAuthenticated);
        m_rrFrontend = nullptr;
    }

    if (!m_region.empty() && !m_accountId.empty() && !m_accountKey.empty() && !m_accountDomain.empty())
    {
        setStatus(AccountConnectionStatus::CheckingCredentials);

        RR::AzureFrontendAccountInfo fi;
        memset(&fi, 0, sizeof(RR::AzureFrontendAccountInfo));
        fi.AccountDomain = m_region;
        fi.AccountId = m_accountId;
        fi.AccountKey = m_accountKey;
        fi.AccountAuthenticationDomain = m_accountDomain;

        auto frontend = RR::ApiHandle(RR::AzureFrontend(fi));
        frontend->MessageLogged(&qArrSdkMessage);
        frontend->SetLogLevel(RR::LogLevel::Debug);
        QPointer<ArrFrontend> thisPtr = this;
        auto async = frontend->GetCurrentRenderingSessionsAsync();
        if (async)
        {
            m_sessionPropertiesAsync = async.value();
            m_sessionPropertiesAsync->Completed([thisPtr, frontend](const RR::ApiHandle<RR::SessionPropertiesArrayAsync>& /*async*/) {
                {
                    std::unique_lock<std::mutex> lk(thisPtr->m_mutex);
                    thisPtr->m_condVar.notify_one();
                }
                QMetaObject::invokeMethod(QApplication::instance(),
                                          [thisPtr, frontend]() {
                                              if (thisPtr != nullptr)
                                              {
                                                  auto status = thisPtr->m_sessionPropertiesAsync->GetStatus();
                                                  if (status == RR::Result::Success && thisPtr->m_sessionPropertiesAsync->GetContext().Result == RR::Result::Success)
                                                  {
                                                      thisPtr->m_rrFrontend = frontend;
                                                      thisPtr->setStatus(AccountConnectionStatus::Authenticated);
                                                  }
                                                  else
                                                  {
                                                      thisPtr->setStatus(AccountConnectionStatus::InvalidCredentials);
                                                  }
                                                  thisPtr->m_sessionPropertiesAsync = nullptr;
                                              }
                                          });
            });
        }
        else
        {
            qWarning(LoggingCategory::configuration) << tr("Failed to get rendering sessions. Possibly invalid account data.");
        }
    }

#ifndef NDEBUG
    m_reentryCnt.fetch_sub(1);
#endif
}

ArrFrontend::~ArrFrontend()
{
    // Wait for current rendering sessions query to complete.
    std::unique_lock<std::mutex> lk(m_mutex);
    if (m_sessionPropertiesAsync)
    {
        auto asyncStatus = m_sessionPropertiesAsync->GetStatus();
        if (asyncStatus == RR::Result::InProgress)
        {
            m_condVar.wait(lk);
        }
    }

    m_rrFrontend = nullptr;
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
