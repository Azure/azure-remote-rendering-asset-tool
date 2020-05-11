#include <Model/ArrFrontend.h>
#include <Model/ArrSessionManager.h>
#include <Model/Configuration.h>
#include <Model/Log/LogHelpers.h>
#include <Model/Settings/VideoSettings.h>
#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <ViewModel/ModelEditor/ViewportModel.h>
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
    const std::chrono::milliseconds s_updateTime = 5s;
    // interval to call the update function in the remote rendering client. See Microsoft::Azure::RemoteRendering::Internal::RemoteRenderingClient::Update()
    const std::chrono::milliseconds s_updateClientTime = 100ms;
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
    QObject::connect(m_updateTimer, &QTimer::timeout, this, [this]() { updateStatus(); });
    m_updateTimer->start(s_updateTime);

    m_clientUpdateTimer = new QTimer(this);
    QObject::connect(m_clientUpdateTimer, &QTimer::timeout, this, [this]() {
        if (m_api)
        {
            m_api->Update();
        }
    });
    m_clientUpdateTimer->start(s_updateClientTime);

    QObject::connect(configuration->getVideoSettings(), &VideoSettings::changed, this, [this]() {
        m_waitForVideoFormatChange = false;
    });

    try
    {
        m_viewportModel = new ViewportModel(m_configuration->getVideoSettings(), m_configuration->getCameraSettings(), this);
    }
    catch (...)
    {
        qFatal("Viewport couldn't be initialized");
    }

    QObject::connect(m_frontend, &ArrFrontend::onStatusChanged, this,
                     [this]() {
                         const bool enabled = m_frontend->getStatus() == AccountConnectionStatus::Connected;
                         if (m_isEnabled != enabled)
                         {
                             m_isEnabled = enabled;
                             if (m_isEnabled)
                             {
                                 if (!m_configuration->getRunningSession().empty())
                                 {
                                     qInfo(LoggingCategory::renderingSession)
                                         << tr("Trying to connect to the saved running session:") << m_configuration->getRunningSession();
                                     setRunningSession(m_frontend->getFrontend().OpenRenderingSession(m_configuration->getRunningSession()));
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
    std::shared_ptr<RR::CreateSessionAsync> async = m_startRequested = m_frontend->getFrontend().CreateNewRenderingSessionAsync(info);
    async->Completed([thisPtr](const std::shared_ptr<Microsoft::Azure::RemoteRendering::CreateSessionAsync>& completedAsync) {
        logContext(completedAsync->Context());
        const auto result = completedAsync->Status();
        if (result == RR::Result::Success)
        {
            QMetaObject::invokeMethod(QApplication::instance(), [thisPtr, session = completedAsync->Result(), result] {
                if (thisPtr)
                {
                    thisPtr->m_startRequested = nullptr;
                    if (result == RR::Result::Success)
                    {
                        thisPtr->setRunningSession(session);
                    }
                    else
                    {
                        qWarning(LoggingCategory::renderingSession)
                            << tr("Failed to request creation of session. Failure reason:") << result;
                    }
                }
            });
        }
    });

    if (async->Status() != RR::Result::InProgress && async->Status() != RR::Result::Success)
    {
        qWarning(LoggingCategory::renderingSession)
            << tr("Failed to request creation of session. Failure reason:") << async->Status();
        m_startRequested = nullptr;
        return false;
    }
    else
    {
        return true;
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
        << tr("Requesting to stop session:\n") << m_session->SessionUUID();
    auto async = m_stopRequested = m_session->StopAsync();
    async->Completed([thisPtr](const std::shared_ptr<RR::SessionAsync>& finishedAsync) {
        logContext(finishedAsync->Context());
        QMetaObject::invokeMethod(QApplication::instance(), [thisPtr, result = finishedAsync->Status()]() {
            if (thisPtr)
            {
                thisPtr->m_stopRequested = nullptr;
                thisPtr->updateStatus();
            }
        });
    });
    if (async->Status() != RR::Result::InProgress || async->Status() != RR::Result::Success)
    {
        m_stopRequested = nullptr;
        qWarning(LoggingCategory::renderingSession)
            << tr("Failed requesting session stop. Failure reason:") << async->Status();
        return false;
    }
    else
    {
        return true;
    }
}

SessionStatus ArrSessionManager::getSessionStatus() const
{
    SessionStatus status = {};

    if (m_session)
    {
        const SessionStatus::Status s = convertStatus(m_lastProperties.Status);

        if (s == SessionStatus::Status::ReadyConnected)
        {
            auto connectionStatus = m_session->ConnectionStatus();

            if (m_stopRequested)
            {
                status.m_status = SessionStatus::Status::StopRequested;
            }
            else
            {
                switch (connectionStatus)
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
            << tr("Requesting extension of session time. Session id:") << m_session->SessionUUID()
            << "\n"
            << params;
        m_renewAsync = m_session->RenewAsync(params);
        m_renewAsync->Completed([thisPtr](const std::shared_ptr<RR::SessionAsync>& async) {
            logContext(async->Context());
            QMetaObject::invokeMethod(QApplication::instance(), [thisPtr] {
                if (thisPtr)
                {
                    thisPtr->updateStatus();
                    thisPtr->m_renewAsync = nullptr;
                }
            });
        });

        if (m_renewAsync == nullptr || (m_renewAsync->Status() != RR::Result::InProgress && m_renewAsync->Status() != RR::Result::Success))
        {
            qWarning(LoggingCategory::renderingSession)
                << tr("Failed requesting extension of session time. Failure reason:") << (m_renewAsync != nullptr ? m_renewAsync->Status() : RR::Result::Fail);
            m_renewAsync = nullptr;
            return false;
        }
        else
        {
            return true;
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
            << tr("Requesting session properties. Session id:") << m_session->SessionUUID();
        m_getPropertiesAsync = m_session->GetPropertiesAsync();
        m_getPropertiesAsync->Completed([thisPtr](const std::shared_ptr<RR::SessionPropertiesAsync>& async) {
            logContext(async->Context());
            if (async->Status() == RR::Result::Success)
            {
                QMetaObject::invokeMethod(QApplication::instance(), [thisPtr, props = async->Result()] {
                    if (thisPtr)
                    {
                        thisPtr->updateSessionProperties(props);
                        thisPtr->onStatusUpdated();
                    }
                });
            }
        });
        if (m_getPropertiesAsync->Status() != RR::Result::Success && m_getPropertiesAsync->Status() != RR::Result::InProgress)
        {
            qWarning(LoggingCategory::renderingSession)
                << tr("Failed requesting extension of session time. Failure reason:") << m_getPropertiesAsync->Status();
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
        m_viewportModel->setClient(m_api.get());

        const auto descriptor = getSessionDescriptor();

        m_connectingElapsedTime.start();
        QPointer<ArrSessionManager> thisPtr = this;

        qInfo(LoggingCategory::renderingSession)
            << tr("Requesting connection to session id:") << m_session->SessionUUID();
        m_connecting = m_session->ConnectToRuntime({RR::ServiceRenderMode::DepthBasedComposition, false});
        m_connecting->Completed([thisPtr](const std::shared_ptr<RR::ConnectToRuntimeAsync>& res) {
            if (res->Status() != RR::Result::Success)
            {
                qWarning(LoggingCategory::renderingSession)
                    << tr("Connection failed. Failure reason:") << res->Status();
            }
            else
            {
                qInfo(LoggingCategory::renderingSession)
                    << tr("Connection succeeded.");
            }

            QMetaObject::invokeMethod(QApplication::instance(), [thisPtr, status = res->Status()] {
                if (thisPtr)
                {
                    switch (status)
                    {
                        case RR::Result::VideoFormatNotAvailable:
                            thisPtr->m_waitForVideoFormatChange = true;
                            thisPtr->m_viewportModel->setClient({});
                        default:
                            break;
                    }

                    thisPtr->m_connecting = nullptr;
                    thisPtr->m_lastError = status;
                }
            });
        });

        if (m_connecting->Status() != RR::Result::InProgress && m_connecting->Status() != RR::Result::Success)
        {
            qWarning(LoggingCategory::renderingSession)
                << tr("Failed requesting connection to session. Failure reason:") << m_connecting->Status();
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
            m_clientUpdateTimer->stop();
            m_viewportModel->setClient(nullptr);
            m_clientUpdateTimer->start();
            m_reconnecting = false;
        }
        connectToSessionRuntime();
    }
    if (m_connecting && m_connectingElapsedTime.elapsed() > s_connectionTimeout.count())
    {
        QString timeoutMsg = tr("Connection timed out [%1 milliseconds].").arg(s_connectionTimeout.count());
        if (m_session)
        {
            qWarning(LoggingCategory::renderingSession) << timeoutMsg << tr("Session id:") << m_session->SessionUUID();
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

    Q_EMIT changed();
}

void ArrSessionManager::initializeSession()
{
    QPointer<ArrSessionManager> thisPtr = this;
    m_api->LogLevel(RR::LogLevel::Debug);
    m_messageLoggedToken = m_api->MessageLogged(&qArrSdkMessage);
    m_statusChangedToken = m_session->ConnectionStatusChanged([thisPtr](RR::ConnectionStatus status, RR::Result error) {
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
}

void ArrSessionManager::deinitializeSession()
{
    unloadModel();
    if (m_api)
    {
        m_api->MessageLogged(m_messageLoggedToken);
    }
    m_session->DisconnectFromRuntime();
    m_viewportModel->setClient(nullptr);
    m_session->ConnectionStatusChanged(m_statusChangedToken);
}

void ArrSessionManager::setRunningSession(const std::shared_ptr<RR::AzureSession>& session)
{
    if (m_session != session)
    {
        Q_EMIT sessionAboutToChange();
        if (m_session)
        {
            m_configuration->setRunningSession({});
            deinitializeSession();
        }
        m_api = nullptr;
        m_session = session;

        if (m_session)
        {
            m_api = std::make_shared<RR::RemoteManager>(m_session->Handle());
            initializeSession();
            m_configuration->setRunningSession(session->SessionUUID());
        }
        Q_EMIT sessionChanged();
        updateStatus();
    }
}

RR::Result ArrSessionManager::loadModelAsync(const QString& modelName, const char* assetSAS, RR::LoadResult result, RR::LoadProgress progressCallback)
{
    if (!m_session)
    {
        return RR::Result::ApiUnavailable;
    }
    else
    {
        QPointer<ArrSessionManager> thisPtr = this;

        qInfo(LoggingCategory::renderingSession)
            << tr("Requesting loading of model %1 (SAS=%2)").arg(modelName).arg(assetSAS);
        RR::LoadModelFromSASParams params;
        memset(&params, 0, sizeof(RR::LoadModelFromSASParams));
        params.ModelUrl = assetSAS;
        try
        {
            m_loadModelAsync = m_api->LoadModelFromSASAsync(params);
        }
        catch (...)
        {
            qWarning(LoggingCategory::renderingSession)
                << tr("Loading of model failed. Possibly invalid URL");
            result(RR::Result::Fail, nullptr);
            m_loadModelAsync = nullptr;
            return RR::Result::Fail;
        }
        // Completed lambda is called from the GUI thread
        m_loadModelAsync->Completed([thisPtr, modelName, result(std::move(result))](const std::shared_ptr<RR::LoadModelAsync>& finishedAsync) {
            if (finishedAsync->Status() != RR::Result::Success)
            {
                qWarning(LoggingCategory::renderingSession)
                    << tr("Loading of model failed. Failure reason: ") << finishedAsync->Status();
                result(finishedAsync->Status(), nullptr);
            }
            else
            {
                qInfo(LoggingCategory::renderingSession)
                    << tr("Loading of model succeeded.");
                const auto loadResult = finishedAsync->Result();
                thisPtr->m_modelName = modelName;
                thisPtr->setLoadedModel(loadResult);
                result(finishedAsync->Status(), loadResult->Root());
            }
        });
        m_loadModelAsync->ProgressUpdated([thisPtr, progressCallback = std::move(progressCallback)](float loadingProgress) {
            if (thisPtr)
            {
                progressCallback(loadingProgress);
            }
        });
        if (m_loadModelAsync->Status() != RR::Result::Success && m_loadModelAsync->Status() != RR::Result::InProgress)
        {
            qWarning(LoggingCategory::renderingSession)
                << tr("Failed requesting loading of model. Failure reason: ") << m_loadModelAsync->Status();
        }
        return m_loadModelAsync->Status();
    }
    return RR::Result::Success;
}

void ArrSessionManager::setLoadedModel(std::shared_ptr<RR::LoadModelResult> loadResult)
{
    if (m_session)
    {
        if (m_loadedModel != loadResult)
        {
            if (m_loadedModel && m_loadedModel->Root() && m_loadedModel->Root()->Valid())
            {
                m_loadedModel->Root()->Destroy();
            }
        }
    }
    m_loadedModel = std::move(loadResult);
    Q_EMIT rootIdChanged();
}

RR::RemoteManager* ArrSessionManager::getClientApi() const
{
    return m_api.get();
}

void ArrSessionManager::startInspector()
{
    if (m_session && !m_connectToArrInspector)
    {
        qInfo(LoggingCategory::renderingSession)
            << tr("Requesting connection to ARR inspector.\n")
            << m_lastProperties;
        m_connectToArrInspector = m_session->ConnectToArrInspectorAsync(m_lastProperties.Hostname.c_str());
        m_connectToArrInspector->Completed([thisPtr = QPointer(this)](const std::shared_ptr<RR::ArrInspectorAsync>& async) {
            if (thisPtr)
            {
                thisPtr->m_connectToArrInspector = nullptr;
                if (async->Status() != RR::Result::Success)
                {
                    qWarning(LoggingCategory::renderingSession)
                        << tr("Failed to create connection to ARR inspector. Failure reason: ") << async->Status();
                }
                else
                {
                    qInfo(LoggingCategory::renderingSession)
                        << tr("Opening inspector web page: ") << async->Result().c_str();
                    //try and start a browser
                    QDesktopServices::openUrl(QUrl::fromLocalFile(async->Result().c_str()));
                }
            }
        });
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
