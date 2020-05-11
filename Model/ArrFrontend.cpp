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
    ci.toolId = std::string("ARRT");
    RR::StartupRemoteRendering(ci);
}

void ArrFrontend::connectAccount(const char* accountID, const char* accountKey, const char* region)
{
    if (m_region == region &&
        m_accountId == accountID &&
        m_accountKey == accountKey)
    {
        return;
    }

    m_region = region;
    m_accountId = accountID;
    m_accountKey = accountKey;

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
        setStatus(AccountConnectionStatus::Disconnected);
        m_rrFrontend = nullptr;
    }

    if (!m_region.empty() && !m_accountId.empty() && !m_accountKey.empty())
    {
        setStatus(AccountConnectionStatus::Connecting);

        RR::AzureFrontendAccountInfo fi;
        memset(&fi, 0, sizeof(RR::AzureFrontendAccountInfo));
        fi.AccountDomain = m_region + ".mixedreality.azure.com";
        fi.AccountId = m_accountId;
        fi.AccountKey = m_accountKey;

        auto frontend = std::make_shared<RR::AzureFrontend>(fi);
        frontend->MessageLogged(&qArrSdkMessage);
        frontend->LogLevel(RR::LogLevel::Debug);
        QPointer<ArrFrontend> thisPtr = this;
        std::shared_ptr<RR::SessionPropertiesArrayAsync> async = frontend->GetCurrentRenderingSessionsAsync();
        m_sessionPropertiesAsync = async;
        async->Completed([thisPtr, frontend](const std::shared_ptr<Microsoft::Azure::RemoteRendering::SessionPropertiesArrayAsync>& /*async*/) {
            {
                std::unique_lock<std::mutex> lk(thisPtr->m_mutex);
                thisPtr->m_condVar.notify_one();
            }
            QMetaObject::invokeMethod(QApplication::instance(),
                                      [thisPtr, frontend]() {
                                          if (thisPtr != nullptr)
                                          {
                                              const RR::Result result = thisPtr->m_sessionPropertiesAsync->Status();
                                              const RR::SessionGeneralContext context = thisPtr->m_sessionPropertiesAsync->Context();
                                              thisPtr->m_sessionPropertiesAsync = nullptr;
                                              thisPtr->m_rrFrontend = frontend;
                                              thisPtr->setStatus(result == RR::Result::Success && context.Result == RR::Result::Success ? AccountConnectionStatus::Connected : AccountConnectionStatus::FailedToConnect);
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
    if (m_sessionPropertiesAsync && m_sessionPropertiesAsync->Status() == RR::Result::InProgress)
    {
        m_condVar.wait(lk);
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
