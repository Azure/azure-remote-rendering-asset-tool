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
    int minutes(const RR::ARRTimeSpan& span)
    {
        return span.hour * 60 + span.minute;
    }

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

    inline QDebug& operator<<(QDebug& logger, const RR::RenderingSessionCreationParams& info)
    {
        QJsonObject sessionInfo;
        sessionInfo[QLatin1String("max_lease")] = QString("%1:%2:%3").arg(info.MaxLease.hour).arg(info.MaxLease.minute).arg(info.MaxLease.second);
        sessionInfo[QLatin1String("vm_size")] = toString(info.Size);
        return logger << QCoreApplication::tr("Create rendering session info:") << PrettyJson(sessionInfo);
    }

    inline QDebug& operator<<(QDebug& logger, const RR::RenderingSessionUpdateParams& info)
    {
        QJsonObject sessionInfo;
        sessionInfo[QLatin1String("max_lease")] = QString("%1:%2:%3").arg(info.MaxLease.hour).arg(info.MaxLease.minute).arg(info.MaxLease.second);
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
        sessionProperties[QLatin1String("elapsed_time")] = QString("%1:%2:%3").arg(properties.ElapsedTime.hour).arg(properties.ElapsedTime.minute).arg(properties.ElapsedTime.second);
        sessionProperties[QLatin1String("max_lease")] = QString("%1:%2:%3").arg(properties.MaxLease.hour).arg(properties.MaxLease.minute).arg(properties.MaxLease.second);
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
                            auto session = m_frontend->getFrontend()->OpenRenderingSession(m_configuration->getRunningSession());
                            if (session)
                            {
                                setRunningSession(session.value());
                            }
                            else
                            {
                                qWarning(LoggingCategory::renderingSession) << tr("Failed to connect to the saved running session:") << session.error();
                            }
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

bool ArrSessionManager::startSession(const RR::RenderingSessionCreationParams& info)
{
    // don't call again if the previous start request isn't completed or if the session is already running
    if (m_startRequested || getSessionStatus().isRunning())
    {
        return false;
    }
    // session is only reset when starting instead of when the session is stopped, because the session can still be queried when it's not running,
    // and will keep its status (stopped, expired, error etc)
    m_configuration->setRunningSession({});

    qInfo(LoggingCategory::renderingSession)
        << tr("Requesting new session:\n") << info;

    QPointer<ArrSessionManager> thisPtr = this;
    auto async = m_frontend->getFrontend()->CreateNewRenderingSessionAsync(info);
    if (async)
    {
        m_startRequested = async.value();
        m_startRequested->Completed([thisPtr](const RR::ApiHandle<RR::CreateSessionAsync>& completedAsync) {
            const auto status = completedAsync->GetStatus();
            RR::ApiHandle<RR::AzureSession> session;

            if (status == RR::Result::Success)
            {
                logContext(completedAsync->GetContext());
                session = completedAsync->GetResult();
            }

            QMetaObject::invokeMethod(QApplication::instance(), [thisPtr, session, status] {
                if (thisPtr)
                {
                    thisPtr->m_startRequested = nullptr;
                    if (status == RR::Result::Success)
                    {
                        thisPtr->setRunningSession(session);
                    }
                    else
                    {
                        qWarning(LoggingCategory::renderingSession)
                            << tr("Failed to request creation of session. Failure reason:") << status;
                    }
                }
            });
        });
        return true;
    }
    else
    {
        qWarning(LoggingCategory::renderingSession)
            << tr("Failed to request creation of session. Failure reason:") << async.error();
        m_startRequested = nullptr;
        return false;
    }
}

bool ArrSessionManager::stopSession()
{
    // don't call again if the previous stop request isn't completed, or if the status is not running
    if (m_stopRequested || !getSessionStatus().isRunning() || !m_session)
    {
        return false;
    }
    QPointer<ArrSessionManager> thisPtr = this;
    qInfo(LoggingCategory::renderingSession)
        << tr("Requesting to stop session:\n") << getSessionUuid();
    auto async = m_session->StopAsync();
    if (async)
    {
        m_stopRequested = async.value();
        m_stopRequested->Completed([thisPtr](const RR::ApiHandle<RR::SessionAsync>& completedAsync) {
            const auto status = completedAsync->GetStatus();
            if (status == RR::Result::Success)
            {
                logContext(completedAsync->GetContext());
            }
            QMetaObject::invokeMethod(QApplication::instance(), [thisPtr, status]() {
                if (thisPtr)
                {
                    if (status != RR::Result::Success)
                    {
                        qWarning(LoggingCategory::renderingSession)
                            << tr("Failed requesting session stop. Failure reason:") << status;
                    }
                    thisPtr->m_stopRequested = nullptr;
                    thisPtr->updateStatus();
                }
            });
        });
        return true;
    }
    else
    {
        m_stopRequested = nullptr;
        qWarning(LoggingCategory::renderingSession)
            << tr("Failed requesting session stop. Failure reason:") << async.error();
        return false;
    }
}

SessionStatus ArrSessionManager::getSessionStatus() const
{
    SessionStatus status = {};

    if (m_session && m_session->GetValid())
    {
        const SessionStatus::Status s = convertStatus(m_lastProperties.Status);

        if (s == SessionStatus::Status::ReadyConnected)
        {
            if (m_stopRequested)
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

        status.m_elapsedTime = m_lastProperties.ElapsedTime;
        status.m_sessionMessage = m_lastProperties.Message;
    }
    else if (m_startRequested)
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
        descriptor.m_maxLeaseTime = m_lastProperties.MaxLease;
    }
    return descriptor;
}

void ArrSessionManager::setExtensionTime(uint minutesToAdd, bool extendAutomatically)
{
    m_extensionMinutes = minutesToAdd;
    m_extendAutomatically = extendAutomatically;
    changed();
}

void ArrSessionManager::getExtensionTime(uint& outMinutesToAdd, bool& outExtendAutomatically) const
{
    outMinutesToAdd = m_extensionMinutes;
    outExtendAutomatically = m_extendAutomatically;
}

int ArrSessionManager::getRemainingMinutes() const
{
    auto maxTime = getSessionDescriptor().m_maxLeaseTime;
    auto elapsed = getSessionStatus().m_elapsedTime;
    return minutes(maxTime) - minutes(elapsed);
}

bool ArrSessionManager::extendMaxSessionTime()
{
    if (!m_session || m_renewAsync)
    {
        return false;
    }
    else
    {
        RR::ARRTimeSpan maxTime = getSessionDescriptor().m_maxLeaseTime;
        int totalMinutes = maxTime.minute + maxTime.hour * 60;
        totalMinutes += m_extensionMinutes;

        RR::RenderingSessionUpdateParams params;
        params.MaxLease = {totalMinutes / 60, totalMinutes % 60, 0};

        QPointer<ArrSessionManager> thisPtr = this;

        qInfo(LoggingCategory::renderingSession)
            << tr("Requesting extension of session time. Session id:") << getSessionUuid()
            << "\n"
            << params;
        auto async = m_session->RenewAsync(params);
        if (async)
        {
            m_renewAsync = async.value();
            m_renewAsync->Completed([thisPtr](const RR::ApiHandle<RR::SessionAsync>& completedAsync) {
                if (completedAsync->GetStatus() == RR::Result::Success)
                {
                    logContext(completedAsync->GetContext());
                }
                QMetaObject::invokeMethod(QApplication::instance(), [thisPtr] {
                    if (thisPtr)
                    {
                        thisPtr->updateStatus();
                        thisPtr->m_renewAsync = nullptr;
                    }
                });
            });
            return true;
        }
        else
        {
            qWarning(LoggingCategory::renderingSession)
                << tr("Failed requesting extension of session time. Failure reason:") << async.error();
            m_renewAsync = nullptr;
            return false;
        }
    }
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
        auto async = m_session->GetPropertiesAsync();
        if (async)
        {
            m_getPropertiesAsync = async.value();
            m_getPropertiesAsync->Completed([thisPtr](const RR::ApiHandle<RR::SessionPropertiesAsync>& completedAsync) {
                const auto status = completedAsync->GetStatus();
                if (status == RR::Result::Success)
                {
                    logContext(completedAsync->GetContext());
                    auto props = completedAsync->GetResult();
                    QMetaObject::invokeMethod(QApplication::instance(), [thisPtr, props] {
                        if (thisPtr)
                        {
                            thisPtr->updateSessionProperties(props);
                            thisPtr->onStatusUpdated();
                        }
                    });
                }
            });
        }
        else
        {
            qWarning(LoggingCategory::renderingSession)
                << tr("Failed requesting extension of session time. Failure reason:") << async.error();
        }
    }
    else
    {
        onStatusUpdated();
    }
}

void ArrSessionManager::connectToSessionRuntime()
{
    using namespace std::chrono_literals;

    if (!m_connecting)
    {
        m_viewportModel->setSession(m_session);

        const auto descriptor = getSessionDescriptor();

        m_connectingElapsedTime.start();
        QPointer<ArrSessionManager> thisPtr = this;

        qInfo(LoggingCategory::renderingSession)
            << tr("Requesting connection to session id:") << getSessionUuid();
        const auto async = m_session->ConnectToRuntime({RR::ServiceRenderMode::DepthBasedComposition, false});
        if (async)
        {
            m_connecting = async.value();
            m_connecting->Completed([thisPtr](const RR::ApiHandle<RR::ConnectToRuntimeAsync>& completedAsync) {
                const RR::Result status = completedAsync->GetStatus();
                if (status != RR::Result::Success)
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
                            case RR::Result::VideoFormatNotAvailable:
                                thisPtr->m_waitForVideoFormatChange = true;
                                thisPtr->m_viewportModel->setSession(RR::ApiHandle<RR::AzureSession>());
                            default:
                                break;
                        }

                        thisPtr->m_connecting = nullptr;
                        thisPtr->m_lastError = status;
                    }
                });
            });
        }
        else
        {
            qWarning(LoggingCategory::renderingSession)
                << tr("Failed requesting connection to session. Failure reason:") << async.error();
            m_connecting = nullptr;
        }
    }
}

void ArrSessionManager::disconnectFromSessionRuntime()
{
    unloadModel();
    m_session->DisconnectFromRuntime();
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
            m_viewportModel->setSession(RR::ApiHandle<RR::AzureSession>());
            m_reconnecting = false;
        }
        connectToSessionRuntime();
    }
    if (m_connecting && m_connectingElapsedTime.elapsed() > s_connectionTimeout.count())
    {
        QString timeoutMsg = tr("Connection timed out [%1 milliseconds].").arg(s_connectionTimeout.count());
        if (m_session)
        {
            qWarning(LoggingCategory::renderingSession) << timeoutMsg << tr("Session id:") << getSessionUuid();
            m_session->DisconnectFromRuntime();
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
    m_session->GetSessionUUID(sessionUuid);
    return sessionUuid;
}

RR::ApiHandle<RR::AzureSession> ArrSessionManager::getCurrentSession() const
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
    m_session->DisconnectFromRuntime();
    m_viewportModel->setSession(RR::ApiHandle<RR::AzureSession>());
    m_session->ConnectionStatusChanged(m_statusChangedToken);
}

void ArrSessionManager::setRunningSession(const RR::ApiHandle<RR::AzureSession>& session)
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
            m_api = m_session->Actions();
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
        RR::LoadModelFromSASParams params{};
        params.ModelUrl = assetSAS;
        if (auto loadModelAsync = api->LoadModelFromSASAsync(params))
        {
            m_loadModelAsync = loadModelAsync.value();
        }
        else
        {
            qWarning(LoggingCategory::renderingSession)
                << tr("Loading of model failed. Possibly invalid URL");
            result(RR::Result::Fail, nullptr);
            m_loadModelAsync = nullptr;
            return RR::Result::Fail;
        }
        // Completed lambda is called from the GUI thread
        m_loadModelAsync->Completed([thisPtr, modelName, result(std::move(result))](const RR::ApiHandle<RR::LoadModelAsync>& finishedAsync) {
            const auto status = finishedAsync->GetStatus();
            if (status != RR::Result::Success)
            {
                qWarning(LoggingCategory::renderingSession)
                    << tr("Loading of model failed. Failure reason: ") << status;
                result(status, nullptr);
            }
            else
            {
                const auto loadResult = finishedAsync->GetResult();
                if (loadResult)
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
                    result(RR::Result::Fail, nullptr);
                }
            }
        });
        m_loadModelAsync->ProgressUpdated([thisPtr, progressCallback = std::move(progressCallback)](float loadingProgress) {
            if (thisPtr)
            {
                progressCallback(loadingProgress);
            }
        });
        return m_loadModelAsync->GetStatus();
    }
    else
    {
        return RR::Result::ApiUnavailable;
    }
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

RR::ApiHandle<RR::RemoteManager> ArrSessionManager::getClientApi()
{
    return (m_session && m_session->GetValid()) ? m_api : RR::ApiHandle<RR::RemoteManager>();
}

void ArrSessionManager::startInspector()
{
    if (m_session && !m_connectToArrInspector)
    {
        qInfo(LoggingCategory::renderingSession)
            << tr("Requesting connection to ARR inspector.\n")
            << m_lastProperties;
        if (const auto async = m_session->ConnectToArrInspectorAsync())
        {
            m_connectToArrInspector = async.value();
            m_connectToArrInspector->Completed([thisPtr = QPointer(this)](const RR::ApiHandle<RR::ArrInspectorAsync>& async) {
                if (thisPtr)
                {
                    thisPtr->m_connectToArrInspector = nullptr;
                    const auto status = async->GetStatus();
                    if (status != RR::Result::Success)
                    {
                        qWarning(LoggingCategory::renderingSession)
                            << tr("Failed to create connection to ARR inspector. Failure reason: ") << status;
                    }
                    else
                    {
                        std::string result;
                        async->GetResult(result);
                        qInfo(LoggingCategory::renderingSession)
                            << tr("Opening inspector web page: ") << result.c_str();
                        //try and start a browser
                        QDesktopServices::openUrl(QUrl::fromLocalFile(result.c_str()));
                    }
                }
            });
        }
    }
}

void ArrSessionManager::reconnectToSessionRuntime()
{
    if (m_session && !m_connecting)
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