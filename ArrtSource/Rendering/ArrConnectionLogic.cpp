#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <Rendering/ArrConnectionLogic.h>
#include <Utils/Logging.h>

using namespace std::chrono_literals;

// interval to check the status (elapsed time, connection status etc)
const std::chrono::milliseconds s_updatePropertiesFast = 10s;
// interval to check the status (elapsed time, connection status etc) when the session is connected
const std::chrono::milliseconds s_updatePropertiesSlow = 15s;
// timeout for connecting to the azure session, once it's ready
const std::chrono::milliseconds s_connectionTimeout = 20s;

ArrConnectionLogic::ArrConnectionLogic()
{
    m_updateSessionPropertiesTimer = new QTimer(this);
    connect(m_updateSessionPropertiesTimer, &QTimer::timeout, this, [this]()
            { UpdateSessionProperties(); });
}

ArrConnectionLogic::~ArrConnectionLogic()
{
    if (IsConnectionStoppable())
    {
        CloseSession(false);
    }
}

QString ArrConnectionLogic::GetStateString(State state)
{
    switch (state)
    {
        case ArrConnectionLogic::State::Inactive:
            return "Inactive";
        case ArrConnectionLogic::State::Error:
            return "Error";
        case ArrConnectionLogic::State::Stopped:
            return "Stopped";
        case ArrConnectionLogic::State::Expired:
            return "Expired";
        case ArrConnectionLogic::State::OpeningSession:
            return "Opening Session";
        case ArrConnectionLogic::State::SessionOpen:
            return "Session Open";
        case ArrConnectionLogic::State::RuntimeConnecting:
            return "Runtime Connecting";
        case ArrConnectionLogic::State::RuntimeConnected:
            return "Runtime Connected";
        case ArrConnectionLogic::State::Disconnecting:
            return "Disconnecting";
    }

    assert(false);
    return "";
}

bool ArrConnectionLogic::IsConnectionActive() const
{
    switch (m_currentState)
    {
        case State::OpeningSession:
        case State::SessionOpen:
        case State::RuntimeConnecting:
        case State::RuntimeConnected:
        case State::Disconnecting:
            return true;

        case State::Inactive:
        case State::Error:
        case State::Stopped:
        case State::Expired:
            return false;

        default:
            break;
    }

    assert(false);
    return false;
}

bool ArrConnectionLogic::IsConnectionStoppable() const
{
    switch (m_currentState)
    {
        case State::OpeningSession:
        case State::SessionOpen:
        case State::RuntimeConnecting:
        case State::RuntimeConnected:
            return true;

        case State::Inactive:
        case State::Error:
        case State::Disconnecting:
        case State::Stopped:
        case State::Expired:
            return false;

        default:
            break;
    }

    assert(false);
    return false;
}

bool ArrConnectionLogic::IsConnectionRendering() const
{
    return m_currentState == State::RuntimeConnected;
}

void ArrConnectionLogic::CreateNewSession(RR::ApiHandle<RR::RemoteRenderingClient>& client, const RR::RenderingSessionCreationOptions& info)
{
    assert(!IsConnectionActive());
    m_currentState = State::OpeningSession;
    Q_EMIT ConnectionStateChanged();

    auto callOpenOrCreateSessionResult = [this](RR::Status status, RR::ApiHandle<RR::CreateRenderingSessionResult> result)
    {
        // this callback might get called on any worker thread, but we want to proceed on the main thread
        QMetaObject::invokeMethod(QApplication::instance(), [this, status, result]()
                                  { this->OpenOrCreateSessionResult(status, result); });
    };

    client->CreateNewRenderingSessionAsync(info, callOpenOrCreateSessionResult);
}

void ArrConnectionLogic::OpenExistingSession(RR::ApiHandle<RR::RemoteRenderingClient>& client, const QString& sessionID)
{
    assert(!IsConnectionActive());
    m_currentState = State::OpeningSession;
    Q_EMIT ConnectionStateChanged();

    auto callOpenOrCreateSessionResult = [this](RR::Status status, RR::ApiHandle<RR::CreateRenderingSessionResult> result)
    {
        // this callback might get called on any worker thread, but we want to proceed on the main thread
        QMetaObject::invokeMethod(QApplication::instance(), [this, status, result]()
                                  { this->OpenOrCreateSessionResult(status, result); });
    };

    client->OpenRenderingSessionAsync(sessionID.toStdString(), callOpenOrCreateSessionResult);
}

void ArrConnectionLogic::CloseSession(bool keepRunning)
{
    assert(IsConnectionStoppable());

    m_currentState = State::Disconnecting;
    Q_EMIT ConnectionStateChanged();

    // remove the status changed event callback
    if (m_arrConnectionStatusChangedToken)
    {
        m_arrSession->ConnectionStatusChanged(m_arrConnectionStatusChangedToken);
        m_arrConnectionStatusChangedToken.invalidate();
    }

    // stop the update timer
    ConfigureSessionPropertiesUpdateTimer();

    DisconnectFromRuntime();

    if (!keepRunning)
    {
        auto onStop = [](RR::Status, RR::ApiHandle<RR::SessionContextResult>) { /* we could retrieve details here, but there is really no reason why stopping a session should fail, other than if we tried to stop an already stopped or expired session */ };
        m_arrSession->StopAsync(onStop);
    }

    m_arrSession = {};
    m_currentState = State::Stopped;
    Q_EMIT ConnectionStateChanged();
}

RR::ApiHandle<RR::RenderingSession> ArrConnectionLogic::GetArrSession() const
{
    if (m_arrSession && m_arrSession.valid())
        return m_arrSession;

    return {};
}

RR::ApiHandle<RR::RenderingConnection> ArrConnectionLogic::GetArrRenderingConnection()
{
    if (m_arrSession && m_arrSession.valid())
        return m_arrSession->Connection();

    return {};
}

void ArrConnectionLogic::OpenOrCreateSessionResult(RR::Status status, RR::ApiHandle<RR::CreateRenderingSessionResult> result)
{
    RR::Result errorCode = RR::StatusToResult(status);

    if (status == RR::Status::OK)
    {
        // if the call to the service generally succeeded, retrieve details
        // session creation might still have failed, though
        errorCode = result->GetErrorCode();
    }

    if (errorCode == RR::Result::Success)
    {
        // everything went fine -> use the session
        m_arrSession = result->GetSession();

        m_currentState = State::SessionOpen;
        Q_EMIT ConnectionStateChanged();

        std::string sessionUuid;
        m_arrSession->GetSessionUuid(sessionUuid);

        qInfo(LoggingCategory::RenderingSession)
            << "SessionID: " << sessionUuid.c_str();

        auto callOnArrConnectionStatusChanged = [this](RR::ConnectionStatus status, RR::Result result)
        {
            // this callback might get called on any worker thread, but we want to proceed on the main thread
            QMetaObject::invokeMethod(QApplication::instance(), [this, status, result]()
                                      { this->OnArrConnectionStatusChanged(status, result); });
        };

        // we want to be informed whenever the status of the session changes
        m_arrConnectionStatusChangedToken = m_arrSession->ConnectionStatusChanged(callOnArrConnectionStatusChanged).value();
    }
    else
    {
        m_currentState = State::Error;
        Q_EMIT ConnectionStateChanged();

        qCritical(LoggingCategory::RenderingSession)
            << "Session creation failed: " << errorCode;

        qDebug(LoggingCategory::RenderingSession)
            << "Session context:\n"
            << result->GetContext();

        QMessageBox::warning(nullptr, "Session Creation Failed", "Session creation failed.\n\nSee the log for details.", QMessageBox::Ok, QMessageBox::Ok);
    }

    // start or stop the update timer
    ConfigureSessionPropertiesUpdateTimer();

    // kick off a property update
    UpdateSessionProperties();
}

void ArrConnectionLogic::OnArrConnectionStatusChanged(RR::ConnectionStatus /*status*/, RR::Result /*result*/)
{
    UpdateSessionProperties();
}

void ArrConnectionLogic::ConfigureSessionPropertiesUpdateTimer()
{
    enum class UpdateType
    {
        // no update needed. The session is not active
        STOPPED,
        // the session is in a temporary state. Update with higher frequency
        FAST_UPDATE,
        // the session is connected and in a stable state. Update with a slower frequency
        SLOW_UPDATE
    };

    UpdateType oldUpdateType = UpdateType::STOPPED;

    if (m_updateSessionPropertiesTimer->isActive())
    {
        oldUpdateType = m_updateSessionPropertiesTimer->interval() == s_updatePropertiesFast.count() ? UpdateType::FAST_UPDATE : UpdateType::SLOW_UPDATE;
    }

    UpdateType newUpdateType = UpdateType::STOPPED;

    if (IsConnectionStoppable())
    {
        newUpdateType = IsConnectionRendering() ? UpdateType::SLOW_UPDATE : UpdateType::FAST_UPDATE;
    }

    if (oldUpdateType != newUpdateType)
    {
        switch (newUpdateType)
        {
            case UpdateType::STOPPED:
                m_updateSessionPropertiesTimer->stop();
                break;
            case UpdateType::FAST_UPDATE:
                m_updateSessionPropertiesTimer->setInterval(s_updatePropertiesFast);
                m_updateSessionPropertiesTimer->start();
                break;
            case UpdateType::SLOW_UPDATE:
                m_updateSessionPropertiesTimer->setInterval(s_updatePropertiesSlow);
                m_updateSessionPropertiesTimer->start();
                break;
        }
    }
}

void ArrConnectionLogic::UpdateSessionProperties()
{
    // adjust the timer speed
    ConfigureSessionPropertiesUpdateTimer();

    if (!m_arrSession)
        return;

    // already currently updating properties?
    if (m_updateSessionPropertiesInProgress.exchange(true))
        return;

    auto callSessionPropertiesResult = [this](RR::Status status, RR::ApiHandle<RR::RenderingSessionPropertiesResult> result)
    {
        // this callback might get called on any worker thread, but we want to proceed on the main thread
        QMetaObject::invokeMethod(QApplication::instance(), [this, status, result]()
                                  { this->SessionPropertiesResult(status, result); m_updateSessionPropertiesInProgress = false; });
    };

    m_arrSession->GetPropertiesAsync(callSessionPropertiesResult);
}

void ArrConnectionLogic::SessionPropertiesResult(RR::Status status, RR::ApiHandle<RR::RenderingSessionPropertiesResult> result)
{
    RR::Result errorCode = RR::StatusToResult(status);

    if (status == RR::Status::OK)
    {
        // if the call to the service generally succeeded, retrieve details
        errorCode = result->GetErrorCode();
    }

    if (errorCode == RR::Result::Success)
    {
        auto props = result->GetSessionProperties();

        RR::ConnectionStatus connection = RR::ConnectionStatus::Disconnected;

        if (m_arrSession && m_arrSession.valid())
        {
            connection = m_arrSession->GetConnectionStatus();
        }

        UpdateState(props, connection);

        if (m_currentState == State::RuntimeConnecting && m_connectToRuntimeElapsedTime.elapsed() > s_connectionTimeout.count())
        {
            qCritical(LoggingCategory::RenderingSession) << "Failed to connect to session. Timeout reached.";

            DisconnectFromRuntime();

            m_currentState = State::SessionOpen;
            Q_EMIT ConnectionStateChanged();
        }

        if (m_currentState == State::SessionOpen)
        {
            // Try to connect whenever the session is ready
            ConnectToRuntime();
        }

        Q_EMIT ConnectionStateChanged();
    }
    else
    {
        qWarning(LoggingCategory::RenderingSession) << "Retrieving session properties failed: " << errorCode;
    }
}

void ArrConnectionLogic::UpdateState(const RR::RenderingSessionProperties& properties, RR::ConnectionStatus connection)
{
    m_elapsedTimeInMinutes = properties.ElapsedTimeInMinutes;
    m_leaseTimeInMinutes = properties.MaxLeaseInMinutes;

    if (m_currentState == State::Disconnecting)
    {
        // no state change while we are disconnecting
        return;
    }

    switch (properties.Status)
    {
        case RR::RenderingSessionStatus::Error:
            m_currentState = State::Error;
            return;

        case RR::RenderingSessionStatus::Expired:
            m_currentState = State::Expired;
            return;

        case RR::RenderingSessionStatus::Stopped:
            m_currentState = State::Stopped;
            return;

        case Microsoft::Azure::RemoteRendering::RenderingSessionStatus::Unknown:
        case Microsoft::Azure::RemoteRendering::RenderingSessionStatus::Starting:
            // no state change
            return;

        case Microsoft::Azure::RemoteRendering::RenderingSessionStatus::Ready:
            break;
    }

    if (m_currentState == State::SessionOpen || m_currentState == State::RuntimeConnecting || m_currentState == State::RuntimeConnected)
    {
        switch (connection)
        {
            case RR::ConnectionStatus::Disconnected:
                m_currentState = State::SessionOpen;
                return;

            case RR::ConnectionStatus::Connecting:
                m_currentState = State::RuntimeConnecting;
                return;

            case RR::ConnectionStatus::Connected:
                m_currentState = State::RuntimeConnected;
                return;
        }
    }
}

void ArrConnectionLogic::ConnectToRuntime()
{
    assert(m_currentState == State::SessionOpen);
    m_currentState = State::RuntimeConnecting;
    Q_EMIT ConnectionStateChanged();

    // this is needed to initialize the ARR graphics settings
    Q_EMIT InitGraphics();

    RR::RendererInitOptions options = {RR::ServiceRenderMode::DepthBasedComposition, false};

    auto callConnectToRuntimeResult = [this](RR::Status status, RR::ConnectionStatus result)
    {
        // this callback might get called on any worker thread, but we want to proceed on the main thread
        QMetaObject::invokeMethod(QApplication::instance(), [this, status, result]()
                                  { ConnectToRuntimeResult(status, result); });
    };

    m_arrSession->ConnectAsync(options, callConnectToRuntimeResult);
    m_connectToRuntimeElapsedTime.start();
}

void ArrConnectionLogic::ConnectToRuntimeResult(RR::Status status, RR::ConnectionStatus /*result*/)
{
    if (status == RR::Status::OK)
    {
        m_arrSession->Connection()->SetLogLevel(RR::LogLevel::Information);
        m_messageLoggedToken = m_arrSession->Connection()->MessageLogged(&ForwardArrLogMsgToQt).value();
    }

    UpdateSessionProperties();
}

void ArrConnectionLogic::DisconnectFromRuntime()
{
    Q_EMIT DeinitGraphics();

    if (m_messageLoggedToken)
    {
        if (auto renderConnection = GetArrRenderingConnection())
        {
            renderConnection->MessageLogged(m_messageLoggedToken);
        }

        m_messageLoggedToken.invalidate();
    }

    m_arrSession->Disconnect();
}
