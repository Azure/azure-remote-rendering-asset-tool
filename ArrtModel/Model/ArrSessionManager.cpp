#include <AzureRemoteRendering.h>
#include <Model/ARRServiceStats.h>
#include <Model/ArrFrontend.h>
#include <Model/ArrSessionManager.h>
#include <Model/Configuration.h>
#include <Model/Log/LogHelpers.h>
#include <Model/Settings/CameraSettings.h>
#include <Model/Settings/VideoSettings.h>
#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <ViewModel/ModelEditor/Viewport/ViewportModel.h>
#include <ctime>
#include <memory>

using namespace std::chrono_literals;

namespace
{
    // interval to check the status (elapsed time, connection status etc)
    const std::chrono::milliseconds s_updateTime = 10s;
    // interval to check the status (elapsed time, connection status etc) when the session is connected
    const std::chrono::milliseconds s_updateTimeWhenConnected = 30s;
    // timeout for connecting to the azure session, once it's ready
    const std::chrono::milliseconds s_connectionTimeout = 10s;

    QString toString(const RR::ConnectionStatus& connectionStatus)
    {
        switch (connectionStatus)
        {
            case RR::ConnectionStatus::Disconnected:
                return QCoreApplication::tr("Disconnected");
            case RR::ConnectionStatus::Connecting:
                return QCoreApplication::tr("Connecting");
            case RR::ConnectionStatus::Connected:
                return QCoreApplication::tr("Connected");
        }
        return QCoreApplication::tr("ConnectionStatus:Unknown");
    }

    QString toString(const RR::RenderingSessionStatus& sessionStatus)
    {
        switch (sessionStatus)
        {
            case RR::RenderingSessionStatus::Unknown:
                return QCoreApplication::tr("Unknown");
            case RR::RenderingSessionStatus::Starting:
                return QCoreApplication::tr("Starting");
            case RR::RenderingSessionStatus::Ready:
                return QCoreApplication::tr("Ready");
            case RR::RenderingSessionStatus::Stopped:
                return QCoreApplication::tr("Stopped");
            case RR::RenderingSessionStatus::Expired:
                return QCoreApplication::tr("Expired");
            case RR::RenderingSessionStatus::Error:
                return QCoreApplication::tr("Error");
        }
        return QCoreApplication::tr("SessionStatus:Unknown");
    }

    QString toString(const RR::RenderingSessionVmSize& vmSize)
    {
        switch (vmSize)
        {
            case RR::RenderingSessionVmSize::None:
                return QCoreApplication::tr("None");
            case RR::RenderingSessionVmSize::Premium:
                return QCoreApplication::tr("Premium");
            case RR::RenderingSessionVmSize::Standard:
                return QCoreApplication::tr("Standard");
        }
        return QCoreApplication::tr("Unknown");
    }

    inline QDebug& operator<<(QDebug& logger, const RR::ConnectionStatus& connectionStatus)
    {
        return logger << toString(connectionStatus);
    }

    inline QDebug& operator<<(QDebug& logger, const RR::RenderingSessionCreationOptions& info)
    {
        QJsonObject sessionInfo;
        sessionInfo[QLatin1String("max_lease")] = QString("%1:%2").arg(info.MaxLeaseInMinutes / 60).arg(info.MaxLeaseInMinutes % 60);
        sessionInfo[QLatin1String("vm_size")] = toString(info.Size);
        return logger << QCoreApplication::tr("Create rendering session info:") << PrettyJson(sessionInfo);
    }

    inline QDebug& operator<<(QDebug& logger, const RR::RenderingSessionUpdateOptions& info)
    {
        QJsonObject sessionInfo;
        sessionInfo[QLatin1String("max_lease")] = QString("%1:%2").arg(info.MaxLeaseInMinutes / 60).arg(info.MaxLeaseInMinutes % 60);
        return logger << QCoreApplication::tr("Update rendering session info:") << PrettyJson(sessionInfo);
    }

    inline QDebug& operator<<(QDebug& logger, const RR::RenderingSessionProperties& properties)
    {
        QJsonObject sessionProperties;
        sessionProperties[QLatin1String("status")] = toString(properties.Status);
        sessionProperties[QLatin1String("size")] = toString(properties.Size);
        sessionProperties[QLatin1String("hostname")] = QString::fromStdString(properties.Hostname);
        sessionProperties[QLatin1String("message")] = QString::fromStdString(properties.Message);
        sessionProperties[QLatin1String("size_string")] = QString::fromStdString(properties.SizeString);
        sessionProperties[QLatin1String("id")] = QString::fromStdString(properties.Id);
        sessionProperties[QLatin1String("elapsed_time")] = QString("%1:%2").arg(properties.ElapsedTimeInMinutes / 60).arg(properties.ElapsedTimeInMinutes % 60);
        sessionProperties[QLatin1String("max_lease")] = QString("%1:%2").arg(properties.MaxLeaseInMinutes / 60).arg(properties.MaxLeaseInMinutes % 60);
        return logger << QCoreApplication::tr("Session properties:") << PrettyJson(sessionProperties);
    }

    void logContext(const RR::SessionGeneralContext& context)
    {
        if (context.Result != RR::Result::Success)
        {
            qWarning(LoggingCategory::renderingSession)
                << QCoreApplication::tr("Request failed:\n") << context;
        }
        else
        {
            qDebug(LoggingCategory::renderingSession)
                << QCoreApplication::tr("Request succeeded:\n") << context;
        }
    }
} // namespace

ArrSessionManager::ArrSessionManager(ArrFrontend* frontEnd, Configuration* configuration, QObject* parent)
    : QObject(parent)
    , m_frontend(frontEnd)
    , m_configuration(configuration)
{
    m_extensionMinutes = m_configuration->getUiState("sessionManager:extension", 10);
    m_extendAutomatically = m_configuration->getUiState("sessionManager:extendAutomatically", true);

    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(s_updateTime);
    connect(m_updateTimer, &QTimer::timeout, this, [this]() { updateStatus(); });

    connect(m_configuration->getVideoSettings(), &VideoSettings::changed, this, [this]() {
        m_waitForVideoFormatChange = false;
    });

    connect(m_configuration->getCameraSettings(), &CameraSettings::changed, this, [this] {
        if (auto api = getClientApi())
        {
            api->GetCameraSettings()->SetNearAndFarPlane(m_configuration->getCameraSettings()->getNearPlane(), m_configuration->getCameraSettings()->getFarPlane());
        }
    });

    try
    {
        m_viewportModel = new ViewportModel(m_configuration->getVideoSettings(), m_configuration->getCameraSettings(), this, this);
    }
    catch (...)
    {
        qFatal("Viewport couldn't be initialized");
    }

    connect(m_viewportModel, &ViewportModel::onRefresh, this, [this]() {
        if (auto api = getClientApi())
        {
            api->Update();
        }
    });

    m_serviceStats = new ArrServiceStats(this);
    connect(m_viewportModel, &ViewportModel::onRefresh, this,
            [this]() {
                if (m_session)
                {
                    m_serviceStats->update(m_session);
                }
            });

    connect(m_frontend, &ArrFrontend::onStatusChanged, this,
            [this]() {
                const bool enabled = m_frontend->getStatus() == AccountConnectionStatus::Authenticated;
                if (m_isEnabled != enabled)
                {
                    m_isEnabled = enabled;
                    if (m_isEnabled)
                    {
                        if (!m_configuration->getRunningSession().empty())
                        {
                            qInfo(LoggingCategory::renderingSession)
                                << tr("Trying to connect to the saved running session:") << m_configuration->getRunningSession();

                            QPointer<ArrSessionManager> thisPtr = this;
                            m_frontend->getClient()->OpenRenderingSessionAsync(m_configuration->getRunningSession(), [thisPtr](RR::Status status, RR::ApiHandle<RR::CreateRenderingSessionResult> result) {
                                RR::ApiHandle<RR::RenderingSession> session;
                                std::string errorStr;
                                if (status == RR::Status::OK)
                                {
                                    if (result->GetErrorCode() == RR::Result::Success)
                                    {
                                        session = result->GetSession();
                                    }
                                    else
                                    {
                                        errorStr = result->GetContext().ErrorMessage;
                                    }
                                }
                                else
                                {
                                    errorStr = tr("General failure").toStdString();
                                }

                                QMetaObject::invokeMethod(QApplication::instance(), [thisPtr, session, errorStr]() {
                                    if (thisPtr)
                                    {
                                        if (session)
                                        {
                                            thisPtr->setRunningSession(session);
                                        }
                                        else
                                        {
                                            qWarning(LoggingCategory::renderingSession) << tr("Failed to connect to the saved running session:") << errorStr.c_str();
                                        }
                                    }
                                });
                            });
                        }
                    }
                    else
                    {
                        stopSession();
                        setRunningSession(nullptr);
                    }
                    Q_EMIT onEnabledChanged();
                }
            });
}

ArrSessionManager::~ArrSessionManager()
{
    m_configuration->setUiState("sessionManager:extension", m_extensionMinutes);
    m_configuration->setUiState("sessionManager:extendAutomatically", m_extendAutomatically);
    if (m_session)
    {
        deinitializeSession();
        m_session = nullptr;
    }
}

bool ArrSessionManager::startSession(const RR::RenderingSessionCreationOptions& info)
{
    // don't call again if the previous start request isn't completed or if the session is already running
    if (m_createSessionInProgress || getSessionStatus().isRunning())
    {
        return false;
    }
    // session is only reset when starting instead of when the session is stopped, because the session can still be queried when it's not running,
    // and will keep its status (stopped, expired, error etc)
    m_configuration->setRunningSession({});

    qInfo(LoggingCategory::renderingSession)
        << tr("Requesting new session:\n") << info;

    QPointer<ArrSessionManager> thisPtr = this;
    m_createSessionInProgress = true;
    m_frontend->getClient()->CreateNewRenderingSessionAsync(info, [thisPtr](RR::Status status, RR::ApiHandle<RR::CreateRenderingSessionResult> result) {
        RR::ApiHandle<RR::RenderingSession> session;
        RR::Result resultCode = RR::StatusToResult(status);
        if (status == RR::Status::OK)
        {
            resultCode = result->GetErrorCode();
            logContext(result->GetContext());
            session = result->GetSession();
        }

        QMetaObject::invokeMethod(QApplication::instance(), [thisPtr, session, resultCode] {
            if (thisPtr)
            {
                if (resultCode == RR::Result::Success)
                {
                    thisPtr->setRunningSession(session);
                }
                else
                {
                    qWarning(LoggingCategory::renderingSession)
                        << tr("Failed to request creation of session. Failure reason:") << resultCode;
                }
                thisPtr->m_createSessionInProgress = false;
            }
        });
    });
    return true;
}

bool ArrSessionManager::stopSession()
{
    // don't call again if the previous stop request isn't completed, or if the status is not running
    if (m_stopRequestInProgress || !getSessionStatus().isRunning() || !m_session)
    {
        return false;
    }
    QPointer<ArrSessionManager> thisPtr = this;
    qInfo(LoggingCategory::renderingSession)
        << tr("Requesting to stop session:\n") << getSessionUuid();

    m_stopRequestInProgress = true;
    m_session->StopAsync([thisPtr](RR::Status status, RR::ApiHandle<RR::SessionContextResult> result) {
        RR::Result errorCode = RR::StatusToResult(status);
        if (status == RR::Status::OK)
        {
            errorCode = result->GetErrorCode();
            logContext(result->GetContext());
        }
        QMetaObject::invokeMethod(QApplication::instance(), [thisPtr, errorCode]() {
            if (thisPtr)
            {
                if (errorCode != RR::Result::Success)
                {
                    qWarning(LoggingCategory::renderingSession)
                        << tr("Failed requesting session stop. Failure reason:") << errorCode;
                }
                thisPtr->updateStatus();
                thisPtr->m_stopRequestInProgress = false;
            }
        });
    });

    return true;
}

SessionStatus ArrSessionManager::getSessionStatus() const
{
    SessionStatus status = {};

    if (m_session && m_session->GetValid())
    {
        const SessionStatus::Status s = convertStatus(m_lastProperties.Status);

        if (s == SessionStatus::Status::ReadyConnected)
        {
            if (m_stopRequestInProgress)
            {
                status.m_status = SessionStatus::Status::StopRequested;
            }
            else
            {
                switch (m_session->GetConnectionStatus())
                {
                    case RR::ConnectionStatus::Disconnected:
                        status.m_status = SessionStatus::Status::ReadyNotConnected;
                        break;
                    case RR::ConnectionStatus::Connecting:
                        status.m_status = SessionStatus::Status::ReadyConnecting;
                        break;
                    case RR::ConnectionStatus::Connected:
                        status.m_status = SessionStatus::Status::ReadyConnected;
                        break;
                }
            }
        }
        else
        {
            status.m_status = s;
        }

        status.m_elapsedTimeInMinutes = m_lastProperties.ElapsedTimeInMinutes;
        status.m_sessionMessage = m_lastProperties.Message;
    }
    else if (m_createSessionInProgress)
    {
        status.m_status = SessionStatus::Status::StartRequested;
    }
    return status;
}

SessionDescriptor ArrSessionManager::getSessionDescriptor() const
{
    SessionDescriptor descriptor = {};

    if (m_session)
    {
        descriptor.m_hostName = m_lastProperties.Hostname;
        descriptor.m_size = m_lastProperties.Size;
        descriptor.m_maxLeaseTimeInMinutes = m_lastProperties.MaxLeaseInMinutes;
    }
    return descriptor;
}

void ArrSessionManager::setExtensionTime(uint minutesToAdd, bool extendAutomatically)
{
    m_extensionMinutes = minutesToAdd;
    m_extendAutomatically = extendAutomatically;
    Q_EMIT changed();
}

void ArrSessionManager::getExtensionTime(uint& outMinutesToAdd, bool& outExtendAutomatically) const
{
    outMinutesToAdd = m_extensionMinutes;
    outExtendAutomatically = m_extendAutomatically;
}

int ArrSessionManager::getRemainingMinutes() const
{
    auto maxTime = getSessionDescriptor().m_maxLeaseTimeInMinutes;
    auto elapsed = getSessionStatus().m_elapsedTimeInMinutes;
    return maxTime - elapsed;
}

bool ArrSessionManager::extendMaxSessionTime()
{
    if (!m_session || m_renewAsyncInProgress)
    {
        return false;
    }
    else
    {
        int totalMinutes = getSessionDescriptor().m_maxLeaseTimeInMinutes;
        totalMinutes += m_extensionMinutes;

        RR::RenderingSessionUpdateOptions params;
        params.MaxLeaseInMinutes = totalMinutes;

        QPointer<ArrSessionManager> thisPtr = this;

        qInfo(LoggingCategory::renderingSession)
            << tr("Requesting extension of session time. Session id:") << getSessionUuid()
            << "\n"
            << params;

        m_renewAsyncInProgress = true;
        m_session->RenewAsync(params, [thisPtr](RR::Status status, RR::ApiHandle<RR::SessionContextResult> result) {
            RR::Result errorCode = RR::StatusToResult(status);
            if (status == RR::Status::OK)
            {
                errorCode = result->GetErrorCode();
                logContext(result->GetContext());
            }

            if (errorCode != RR::Result::Success)
            {
                qWarning(LoggingCategory::renderingSession)
                    << tr("Failed requesting extension of session time. Failure reason:") << errorCode;
            }
            QMetaObject::invokeMethod(QApplication::instance(), [thisPtr] {
                if (thisPtr)
                {
                    thisPtr->updateStatus();
                }
            });
            if (thisPtr)
            {
                thisPtr->m_renewAsyncInProgress = false;
            }
        });
    }
    return true;
}


SessionStatus::Status ArrSessionManager::convertStatus(RR::RenderingSessionStatus status)
{
    SessionStatus::Status s = SessionStatus::Status::NotActive;

    switch (status)
    {
        case RR::RenderingSessionStatus::Unknown:
            s = SessionStatus::Status::NotActive;
            break;
        case RR::RenderingSessionStatus::Starting:
            s = SessionStatus::Status::Starting;
            break;
        case RR::RenderingSessionStatus::Ready:
            s = SessionStatus::Status::ReadyConnected;
            break;
        case RR::RenderingSessionStatus::Stopped:
            s = SessionStatus::Status::Stopped;
            break;
        case RR::RenderingSessionStatus::Expired:
            s = SessionStatus::Status::Expired;
            break;
        case RR::RenderingSessionStatus::Error:
            s = SessionStatus::Status::Error;
            break;
    }
    return s;
}

void ArrSessionManager::updateStatus()
{
    if (m_session)
    {
        QPointer<ArrSessionManager> thisPtr = this;

        qDebug(LoggingCategory::renderingSession)
            << tr("Requesting session properties. Session id:") << getSessionUuid();

        m_session->GetPropertiesAsync([thisPtr](RR::Status status, RR::ApiHandle<RR::RenderingSessionPropertiesResult> result) {
            RR::Result errorCode = RR::StatusToResult(status);
            if (status == RR::Status::OK)
            {
                errorCode = result->GetErrorCode();
                logContext(result->GetContext());

                if (errorCode == RR::Result::Success)
                {
                    auto props = result->GetSessionProperties();
                    QMetaObject::invokeMethod(QApplication::instance(), [thisPtr, props] {
                        if (thisPtr)
                        {
                            thisPtr->updateSessionProperties(props);
                            thisPtr->onStatusUpdated();
                        }
                    });
                }
            }

            if (errorCode != RR::Result::Success)
            {
                qWarning(LoggingCategory::renderingSession)
                    << tr("Failed requesting extension of session time. Failure reason:") << errorCode;
            }
        });
    }
    else
    {
        onStatusUpdated();
    }
}

void ArrSessionManager::connectToSessionRuntime()
{
    using namespace std::chrono_literals;

    if (!m_connectingInProgress)
    {
        m_viewportModel->setSession(m_session);

        const auto descriptor = getSessionDescriptor();

        m_connectingElapsedTime.start();
        QPointer<ArrSessionManager> thisPtr = this;

        qInfo(LoggingCategory::renderingSession)
            << tr("Requesting connection to session id:") << getSessionUuid();
        RR::RendererInitOptions options = {RR::ServiceRenderMode::DepthBasedComposition, false};
        m_connectingInProgress = true;
        m_session->ConnectAsync(options, [thisPtr](RR::Status status, RR::ConnectionStatus) {
            if (status != RR::Status::OK)
            {
                qWarning(LoggingCategory::renderingSession)
                    << tr("Connection failed. Failure reason:") << status;
            }
            else
            {
                qInfo(LoggingCategory::renderingSession)
                    << tr("Connection succeeded.");
            }

            QMetaObject::invokeMethod(QApplication::instance(), [thisPtr, status] {
                if (thisPtr)
                {
                    switch (status)
                    {
                        case RR::Status::VideoFormatNotAvailable:
                            thisPtr->m_waitForVideoFormatChange = true;
                            thisPtr->m_viewportModel->setSession(RR::ApiHandle<RR::RenderingSession>());
                        default:
                            break;
                    }

                    thisPtr->m_lastError = status;
                    thisPtr->m_connectingInProgress = false;
                }
            });
        });
    }
}

void ArrSessionManager::disconnectFromSessionRuntime()
{
    unloadModel();
    m_session->Disconnect();
}

namespace
{
    void logSessionStatusUpdate(QDebug&& logger, const RR::RenderingSessionProperties& curProps, const RR::RenderingSessionProperties& newProps)
    {
        logger
            << QCoreApplication::tr("Session status update: %1 -> %2").arg(toString(curProps.Status)).arg(toString(newProps.Status))
            << "\n"
            << newProps;
    }
} // namespace

void ArrSessionManager::updateSessionProperties(RR::RenderingSessionProperties props)
{
    if (m_lastProperties.Status != props.Status)
    {
        if (props.Status == RR::RenderingSessionStatus::Error || props.Status == RR::RenderingSessionStatus::Expired || props.Status == RR::RenderingSessionStatus::Unknown)
        {
            logSessionStatusUpdate(qWarning(LoggingCategory::renderingSession), m_lastProperties, props);
        }
        else
        {
            logSessionStatusUpdate(qInfo(LoggingCategory::renderingSession), m_lastProperties, props);
        }
    }
    m_lastProperties = std::move(props);
}

void ArrSessionManager::onStatusUpdated()
{
    const auto status = getSessionStatus();

    // try and connect whenever the session is ready. It might get stuck on connecting. In that case it reconnects.
    if (!m_waitForVideoFormatChange && status.m_status == SessionStatus::Status::ReadyNotConnected)
    {
        if (m_reconnecting)
        {
            m_viewportModel->setSession(RR::ApiHandle<RR::RenderingSession>());
            m_reconnecting = false;
        }
        connectToSessionRuntime();
    }
    if (m_connectingInProgress && m_connectingElapsedTime.elapsed() > s_connectionTimeout.count())
    {
        QString timeoutMsg = tr("Connection timed out [%1 milliseconds].").arg(s_connectionTimeout.count());
        if (m_session)
        {
            qWarning(LoggingCategory::renderingSession) << timeoutMsg << tr("Session id:") << getSessionUuid();
            m_session->Disconnect();
        }
        else
        {
            qWarning(LoggingCategory::renderingSession) << timeoutMsg << tr("No active session");
        }
    }

    if (status.isRunning() && status.m_status != SessionStatus::Status::StartRequested && m_extendAutomatically && m_extensionMinutes > 0)
    {
        // when automatically extending the session time by X, it will trigger the extension of X when we are X/4 away from expiration
        int minMinutes = (m_extensionMinutes / 4);
        if (minMinutes < 1)
        {
            minMinutes = 1;
        }
        if (getRemainingMinutes() <= minMinutes)
        {
            extendMaxSessionTime();
        }
    }

    // make sure to reflect the fact that a model will be automatically unloaded when the client is not connected
    if (status.m_status != SessionStatus::Status::ReadyConnected)
    {
        setLoadedModel({});
    }

    updateTimers();

    Q_EMIT changed();
}

void ArrSessionManager::initializeSession()
{
    QPointer<ArrSessionManager> thisPtr = this;
    if (auto api = getClientApi())
    {
        api->SetLogLevel(RR::LogLevel::Debug);
        {
            const auto token = api->MessageLogged(&qArrSdkMessage);
            if (token)
            {
                m_messageLoggedToken = token.value();
            }
            else
            {
                qWarning(LoggingCategory::configuration) << tr("Failed to set log handler:") << token.error();
            }
        }
        {
            const auto token = m_session->ConnectionStatusChanged([thisPtr](RR::ConnectionStatus status, RR::Result error) {
                if (error != RR::Result::Success)
                {
                    qWarning(LoggingCategory::renderingSession)
                        << tr("Connection status: ") << status << tr("Failure reason : ") << error;
                }
                else
                {
                    qInfo(LoggingCategory::renderingSession)
                        << tr("Connection status: ") << status;
                }
                QMetaObject::invokeMethod(QApplication::instance(), [thisPtr] {
                    if (thisPtr)
                    {
                        thisPtr->updateStatus();
                    }
                });
            });
            if (token)
            {
                m_statusChangedToken = token.value();
            }
            else
            {
                qWarning(LoggingCategory::configuration) << tr("Failed to set connection statu changed callback:") << token.error();
            }
        }
    }
}

void ArrSessionManager::updateTimers()
{
    enum UpdateType
    {
        // no update needed. The session is not active
        STOPPED,
        // the session is in a temporary state. Update with higher frequency
        FAST_UPDATE,
        // the session is connected and in a stable state. Update with a slower frequency
        SLOW_UPDATE
    };
    UpdateType oldUpdateType;
    if (!m_updateTimer->isActive())
    {
        oldUpdateType = STOPPED;
    }
    else
    {
        oldUpdateType = m_updateTimer->interval() == s_updateTime.count() ? FAST_UPDATE : SLOW_UPDATE;
    }

    UpdateType updateType = STOPPED;
    if (getSessionStatus().isRunning())
    {
        updateType = getSessionStatus().m_status == SessionStatus::Status::ReadyConnected ? SLOW_UPDATE : FAST_UPDATE;
    }

    if (updateType != oldUpdateType)
    {
        switch (updateType)
        {
            case STOPPED:
                m_updateTimer->stop();
                break;
            case FAST_UPDATE:
                m_updateTimer->setInterval(s_updateTime);
                m_updateTimer->start();
                break;
            case SLOW_UPDATE:
                m_updateTimer->setInterval(s_updateTimeWhenConnected);
                m_updateTimer->start();
                break;
        }
    }
}

std::string ArrSessionManager::getSessionUuid() const
{
    if (!m_session && !m_session->GetValid())
    {
        return tr("no session").toStdString();
    }
    std::string sessionUuid;
    m_session->GetSessionUuid(sessionUuid);
    return sessionUuid;
}

RR::ApiHandle<RR::RenderingSession> ArrSessionManager::getCurrentSession() const
{
    return m_session;
}

void ArrSessionManager::deinitializeSession()
{
    unloadModel();
    if (auto api = getClientApi())
    {
        api->MessageLogged(m_messageLoggedToken);
    }
    m_session->Disconnect();
    m_viewportModel->setSession(RR::ApiHandle<RR::RenderingSession>());
    m_session->ConnectionStatusChanged(m_statusChangedToken);
}

void ArrSessionManager::setRunningSession(const RR::ApiHandle<RR::RenderingSession>& session)
{
    if (m_session != session)
    {
        Q_EMIT sessionAboutToChange();
        if (m_session && m_session->GetValid())
        {
            m_configuration->setRunningSession({});
            deinitializeSession();
        }
        m_api = nullptr;
        m_session = session;

        if (m_session && m_session->GetValid())
        {
            m_api = m_session->Connection();
            initializeSession();
            m_configuration->setRunningSession(getSessionUuid());
        }
        Q_EMIT sessionChanged();
        updateStatus();
    }
}

RR::Result ArrSessionManager::loadModelAsync(const QString& modelName, const char* assetSAS, RR::LoadResult result, RR::LoadProgress progressCallback)
{
    if (auto api = getClientApi())
    {
        QPointer<ArrSessionManager> thisPtr = this;

        qInfo(LoggingCategory::renderingSession)
            << tr("Requesting loading of model %1 (SAS=%2)").arg(modelName).arg(assetSAS);
        RR::LoadModelFromSasOptions params{};
        params.ModelUri = assetSAS;
        api->LoadModelFromSasAsync(
            params,
            // completed callback
            [thisPtr, modelName, result(std::move(result))](RR::Status status, RR::ApiHandle<RR::LoadModelResult> loadResult) {
                // Completed lambda is called from the GUI thread
                if (status != RR::Status::OK)
                {
                    qWarning(LoggingCategory::renderingSession)
                        << tr("Loading of model failed. Failure reason: ") << status;
                    result(status, nullptr);
                }
                else
                {
                    if (loadResult.valid())
                    {
                        const auto root = loadResult->GetRoot();
                        qInfo(LoggingCategory::renderingSession)
                            << tr("Loading of model succeeded.");
                        thisPtr->m_modelName = modelName;
                        thisPtr->setLoadedModel(loadResult);
                        result(status, root);
                    }
                    else
                    {
                        qWarning(LoggingCategory::renderingSession)
                            << tr("Loading of model failed.");
                        result(RR::Status::Fail, nullptr);
                    }
                }
            },
            // progress update callback
            [thisPtr, progressCallback = std::move(progressCallback)](float progress) {
                if (thisPtr)
                {
                    progressCallback(progress);
                }
            });

        return RR::Result::Success;
    }

    return RR::Result::ApiUnavailable;
}

void ArrSessionManager::setLoadedModel(RR::ApiHandle<RR::LoadModelResult> loadResult)
{
    if (m_session)
    {
        if (m_loadedModel != loadResult)
        {
            if (m_loadedModel)
            {
                if (auto root = m_loadedModel->GetRoot())
                {
                    root->Destroy();
                }
            }
        }
    }
    m_loadedModel = std::move(loadResult);
    Q_EMIT rootIdChanged();
}

RR::ApiHandle<RR::RenderingConnection> ArrSessionManager::getClientApi()
{
    return (m_session && m_session->GetValid()) ? m_api : RR::ApiHandle<RR::RenderingConnection>();
}

void ArrSessionManager::startInspector()
{
    if (m_session && !m_connectToArrInspectorInProgress)
    {
        qInfo(LoggingCategory::renderingSession)
            << tr("Requesting connection to ARR inspector.\n")
            << m_lastProperties;
        m_connectToArrInspectorInProgress = true;

        m_session->ConnectToArrInspectorAsync([thisPtr = QPointer(this)](RR::Status status, std::string result) {
            if (thisPtr)
            {
                if (status != RR::Status::OK)
                {
                    qWarning(LoggingCategory::renderingSession)
                        << tr("Failed to create connection to ARR inspector. Failure reason: ") << status;
                }
                else
                {
                    qInfo(LoggingCategory::renderingSession)
                        << tr("Opening inspector web page: ") << result.c_str();
                    //try and start a browser
                    QDesktopServices::openUrl(QUrl::fromLocalFile(result.c_str()));
                }

                thisPtr->m_connectToArrInspectorInProgress = false;
            }
        });
    }
}

void ArrSessionManager::reconnectToSessionRuntime()
{
    if (m_session && !m_connectingInProgress)
    {
        disconnectFromSessionRuntime();
        m_reconnecting = true;
    }
}

void ArrSessionManager::unloadModel()
{
    m_modelName.clear();
    setLoadedModel(nullptr);
}

QString ArrSessionManager::getModelName() const
{
    return m_modelName.mid(m_modelName.lastIndexOf(QChar('/')) + 1);
}

bool ArrSessionManager::getAutoRotateRoot() const
{
    return m_autoRotateRoot;
}

void ArrSessionManager::setAutoRotateRoot(bool autoRotateRoot)
{
    if (m_autoRotateRoot != autoRotateRoot)
    {
        m_autoRotateRoot = autoRotateRoot;
        Q_EMIT autoRotateRootChanged();
    }
}