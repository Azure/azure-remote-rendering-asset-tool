#include <QApplication>
#include <QDesktopServices>
#include <QMessageBox>
#include <QPointer>
#include <QTimer>
#include <QUrl>
#include <Rendering/ArrAccount.h>
#include <Rendering/ArrSession.h>
#include <Rendering/ArrSettings.h>
#include <Rendering/UI/SceneState.h>
#include <Utils/Logging.h>
#include <windows.h>

using namespace std::chrono_literals;

// interval to check the status (elapsed time, connection status etc)
const std::chrono::milliseconds s_updatePropertiesFast = 10s;
// interval to check the status (elapsed time, connection status etc) when the session is connected
const std::chrono::milliseconds s_updatePropertiesSlow = 15s;
// timeout for connecting to the azure session, once it's ready
const std::chrono::milliseconds s_connectionTimeout = 10s;

bool ArrSessionStatus::IsRunning() const
{
    switch (m_state)
    {
        case ArrSessionStatus::State::StartRequested:
        case ArrSessionStatus::State::Starting:
        case ArrSessionStatus::State::ReadyNotConnected:
        case ArrSessionStatus::State::ReadyConnecting:
        case ArrSessionStatus::State::ReadyConnected:
            return true;

        default:
            return false;
    }
}

ArrSession::ArrSession(ArrAccount* arrClient, SceneState* sceneState)
{
    m_arrClient = arrClient;
    m_sceneState = sceneState;

    m_updateSessionPropertiesTimer = new QTimer(this);
    connect(m_updateSessionPropertiesTimer, &QTimer::timeout, this, [this]()
            { UpdateSessionProperties(); });

    connect(sceneState, &SceneState::SceneRefreshed, this, [this]()
            { OnSceneRefresh(); });
}

ArrSession::~ArrSession()
{
    CloseSession(false);
}

RR::ApiHandle<RR::RenderingConnection> ArrSession::GetRenderingConnection()
{
    return (m_arrSession && m_arrSession->GetValid()) ? m_arrSession->Connection() : RR::ApiHandle<RR::RenderingConnection>();
}

QString ArrSession::GetSessionID() const
{
    if (!m_arrSession && !m_arrSession->GetValid())
    {
        return "no active session";
    }

    std::string sessionUuid;
    m_arrSession->GetSessionUuid(sessionUuid);
    return sessionUuid.c_str();
}

ArrSessionStatus ArrSession::GetSessionStatus() const
{
    ArrSessionStatus status;

    if (m_arrSession && m_arrSession->GetValid())
    {
        status.m_state = ArrSessionStatus::State::NotActive;

        switch (m_lastProperties.Status)
        {
            case RR::RenderingSessionStatus::Unknown:
                status.m_state = ArrSessionStatus::State::NotActive;
                break;
            case RR::RenderingSessionStatus::Starting:
                status.m_state = ArrSessionStatus::State::Starting;
                break;
            case RR::RenderingSessionStatus::Ready:
                status.m_state = ArrSessionStatus::State::ReadyConnected;
                break;
            case RR::RenderingSessionStatus::Stopped:
                status.m_state = ArrSessionStatus::State::Stopped;
                break;
            case RR::RenderingSessionStatus::Expired:
                status.m_state = ArrSessionStatus::State::Expired;
                break;
            case RR::RenderingSessionStatus::Error:
                status.m_state = ArrSessionStatus::State::Error;
                break;
        }

        if (status.m_state == ArrSessionStatus::State::ReadyConnected)
        {
            switch (m_arrSession->GetConnectionStatus())
            {
                case RR::ConnectionStatus::Disconnected:
                    status.m_state = ArrSessionStatus::State::ReadyNotConnected;
                    break;
                case RR::ConnectionStatus::Connecting:
                    status.m_state = ArrSessionStatus::State::ReadyConnecting;
                    break;
                case RR::ConnectionStatus::Connected:
                    status.m_state = ArrSessionStatus::State::ReadyConnected;
                    break;
            }
        }

        status.m_elapsedTimeInMinutes = m_lastProperties.ElapsedTimeInMinutes;
        status.m_leaseTimeInMinutes = m_lastProperties.MaxLeaseInMinutes;
    }
    else if (m_createSessionInProgress)
    {
        status.m_state = ArrSessionStatus::State::StartRequested;
    }

    return status;
}

void ArrSession::CreateSession(const RR::RenderingSessionCreationOptions& info)
{
    if (m_createSessionInProgress)
        return;

    // this callback might get called on any worker thread
    auto resultCallback = [this](RR::Status status, RR::ApiHandle<RR::CreateRenderingSessionResult> result)
    {
        // invoke the lambda on the main thread, so that it can access Qt elements
        QMetaObject::invokeMethod(QApplication::instance(), [this, status, result]()
                                  { this->OpenOrCreateSessionResult(status, result); });
    };

    m_createSessionInProgress = true;
    m_arrClient->GetClient()->CreateNewRenderingSessionAsync(info, resultCallback);
}

void ArrSession::OpenSession(const QString& sessionID)
{
    if (m_createSessionInProgress)
        return;

    // this callback might get called on any worker thread
    auto resultCallback = [this](RR::Status status, RR::ApiHandle<RR::CreateRenderingSessionResult> result)
    {
        // invoke the lambda on the main thread, so that it can access Qt elements
        QMetaObject::invokeMethod(QApplication::instance(), [this, status, result]()
                                  { this->OpenOrCreateSessionResult(status, result); });
    };

    m_createSessionInProgress = true;
    m_arrClient->GetClient()->OpenRenderingSessionAsync(sessionID.toStdString(), resultCallback);
}

void ArrSession::OpenOrCreateSessionResult(RR::Status status, RR::ApiHandle<RR::CreateRenderingSessionResult> result)
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

        auto resultCallback = [this](RR::ConnectionStatus status, RR::Result result)
        {
            QMetaObject::invokeMethod(QApplication::instance(), [this, status, result]
                                      { OnConnectionStatusChanged(status, result); });
        };

        // we want to be informed whenever the status of the session changes
        m_statusChangedToken = m_arrSession->ConnectionStatusChanged(resultCallback).value();

        UpdateSessionProperties();
    }
    else
    {
        qCritical(LoggingCategory::RenderingSession)
            << "Session creation failed: " << errorCode;

        qDebug(LoggingCategory::RenderingSession)
            << "Session context:\n"
            << result->GetContext();

        // TODO: show details in message box
        QMessageBox::warning(nullptr, "Session Creation Failed", "Session creation failed.\n\nSee the log for details.", QMessageBox::Ok, QMessageBox::Ok);
    }

    m_createSessionInProgress = false;
}

void ArrSession::OnConnectionStatusChanged(RR::ConnectionStatus status, RR::Result result)
{
    if (result != RR::Result::Success)
    {
        if (result == RR::Result::DisconnectRequest)
        {
            qInfo(LoggingCategory::RenderingSession) << "Rendering connection closed by user.";
        }
        else
        {
            qWarning(LoggingCategory::RenderingSession) << "Connection status: " << status << "Reason: " << result;

            if (result == RR::Result::ConnectionLost)
            {
                QString sessionID = GetSessionID();

                qInfo(LoggingCategory::RenderingSession) << "Attempting to reconnect to session " << sessionID;
                qWarning(LoggingCategory::RenderingSession) << "Please be aware that previously loaded models will disappear after a connection loss.";

                CloseSession(true);
                OpenSession(sessionID);
            }
        }
    }

    UpdateSessionProperties();
}

void ArrSession::CloseSession(bool keepRunning)
{
    if (!m_arrSession)
        return;

    DisconnectFromSessionRuntime();

    if (keepRunning == false)
    {
        auto onStop = [](RR::Status, RR::ApiHandle<RR::SessionContextResult>) { /* we could retrieve details here, but there is really no reason why stopping a session should fail, other than if we tried to stop an already stopped or expired session */ };
        m_arrSession->StopAsync(onStop);
    }

    // remove the status changed event callback
    if (m_statusChangedToken)
    {
        m_arrSession->ConnectionStatusChanged(m_statusChangedToken);
        m_statusChangedToken.invalidate();
    }

    m_arrSession = {};

    UpdateSessionProperties();
}

void ArrSession::UpdateSessionProperties()
{
    if (m_arrSession)
    {
        // this callback might get called on any worker thread
        auto resultCallback = [this](RR::Status status, RR::ApiHandle<RR::RenderingSessionPropertiesResult> result)
        {
            // invoke the lambda on the main thread, so that it can access Qt elements
            QMetaObject::invokeMethod(QApplication::instance(), [this, status, result]
                                      { GetSessionPropertiesResult(status, result); });
        };

        m_arrSession->GetPropertiesAsync(resultCallback);
    }
    else
    {
        OnSessionPropertiesUpdated();
    }
}

void ArrSession::GetSessionPropertiesResult(RR::Status status, RR::ApiHandle<RR::RenderingSessionPropertiesResult> result)
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

        if (m_lastProperties.Status != props.Status)
        {
            if (props.Status == RR::RenderingSessionStatus::Error || props.Status == RR::RenderingSessionStatus::Unknown)
            {
                qCritical(LoggingCategory::RenderingSession) << "Session status changed: " << toString(m_lastProperties.Status) << " -> " << toString(props.Status);
            }
            else
            {
                qInfo(LoggingCategory::RenderingSession) << "Session status changed: " << toString(m_lastProperties.Status) << " -> " << toString(props.Status);
            }
        }

        m_lastProperties = props;
        OnSessionPropertiesUpdated();
    }
    else
    {
        qCritical(LoggingCategory::RenderingSession) << "Retrieving session properties failed: " << errorCode;
    }
}

void ArrSession::OnSessionPropertiesUpdated()
{
    const auto status = GetSessionStatus();

    ConfigureSessionPropertiesUpdateTimer();

    // if the connection attempt takes too long, disconnect
    // the code below will then attempt another try to connect
    if (m_connectingInProgress && m_connectingElapsedTime.elapsed() > s_connectionTimeout.count())
    {
        qCritical(LoggingCategory::RenderingSession) << "Connection reached timeout (10 sec). Session ID: " << GetSessionID();

        DisconnectFromSessionRuntime();
    }

    if (!m_connectingInProgress && status.m_state == ArrSessionStatus::State::ReadyNotConnected)
    {
        // Try to connect whenever the session is ready
        ConnectToSessionRuntime();
    }

    if (status.IsRunning())
    {
        ExtendSessionIfNeeded();
    }
    else
    {
        m_loadedModels.clear();
    }

    Q_EMIT SessionStatusChanged();
}

void ArrSession::ConnectToSessionRuntime()
{
    if (m_connectingInProgress)
        return;

    m_connectingInProgress = true;

    // this is needed to initialize the ARR graphics settings
    m_sceneState->SetSession(m_arrSession, this);

    RR::RendererInitOptions options = {RR::ServiceRenderMode::DepthBasedComposition, false};

    // this callback might get called on any worker thread
    auto resultCallback = [this](RR::Status status, RR::ConnectionStatus result)
    {
        // invoke the lambda on the main thread, so that it can access Qt elements
        QMetaObject::invokeMethod(QApplication::instance(), [this, status, result]
                                  { ConnectToSessionRuntimeResult(status, result); });
    };

    m_arrSession->ConnectAsync(options, resultCallback);
    m_connectingElapsedTime.start();
}

void ArrSession::ConnectToSessionRuntimeResult(RR::Status status, RR::ConnectionStatus /*result*/)
{
    if (status != RR::Status::OK)
    {
        m_sceneState->SetSession({}, this);

        qCritical(LoggingCategory::RenderingSession)
            << "Connection failed: " << status;

        // TODO: show details in message box
        QMessageBox::warning(nullptr, "Connection Failed", "Connecting to the rendering session failed.\n\nSee the log for details.", QMessageBox::Ok, QMessageBox::Ok);
    }
    else
    {
        m_arrSession->Connection()->SetLogLevel(RR::LogLevel::Information);
        m_messageLoggedToken = m_arrSession->Connection()->MessageLogged(&ForwardArrLogMsgToQt).value();
    }

    m_connectingInProgress = false;
}

void ArrSession::DisconnectFromSessionRuntime()
{
    if (!m_arrSession)
        return;

    m_sceneState->SetSession({}, nullptr);

    // remove the message event callback
    if (m_messageLoggedToken)
    {
        if (auto renderConnection = GetRenderingConnection())
        {
            renderConnection->MessageLogged(m_messageLoggedToken);
        }

        m_messageLoggedToken.invalidate();
    }

    m_arrSession->Disconnect();

    // cleanup of loaded models
    {
        m_loadingProgress.clear();
        m_loadedModels.clear();
        m_selectedEntities.clear();

        Q_EMIT ModelLoadProgressChanged();
    }
}

void ArrSession::OnSceneRefresh()
{
    if (auto renderConnection = GetRenderingConnection())
    {
        // once per frame we need to tick the ARR rendering connection object
        // this sends out any desired scene updates
        // and locally applies the scene changes coming from the server
        renderConnection->Update();

        UpdatePerformanceStatistics();
    }
}

void ArrSession::ConfigureSessionPropertiesUpdateTimer()
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

    if (GetSessionStatus().IsRunning())
    {
        newUpdateType = GetSessionStatus().m_state == ArrSessionStatus::State::ReadyConnected ? UpdateType::SLOW_UPDATE : UpdateType::FAST_UPDATE;
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

void ArrSession::CheckEntityBounds(RR::ApiHandle<RR::Entity> entity)
{
    entity->QueryWorldBoundsAsync([this](RR::Status status, RR::Bounds bounds)
                                  {
                                      if (status == RR::Status::OK && bounds.IsValid())
                                      {
                                          // pass the information to the main thread, because we want to interact with the GUI
                                          QMetaObject::invokeMethod(QApplication::instance(), [this, bounds]()
                                                                    { CheckEntityBoundsResult(bounds); });
                                      } });
}

void ArrSession::CheckEntityBoundsResult(RR::Bounds bounds)
{
    const float maxX = (float)bounds.Max.X;
    const float minX = (float)bounds.Min.X;
    const float maxY = (float)bounds.Max.Y;
    const float minY = (float)bounds.Min.Y;
    const float maxZ = (float)bounds.Max.Z;
    const float minZ = (float)bounds.Min.Z;

    const float width = maxX - minX;
    const float height = maxY - minY;
    const float depth = maxZ - minZ;

    const float smallSize = 0.1f;
    const float largeSize = 10.0f;
    const float largeOffset = 10.0f;

    const bool isLarge = (width > largeSize || height > largeSize || depth > largeSize);
    const bool isSmall = (width < smallSize || height < smallSize || depth < smallSize);
    const bool isFarAway = minX > largeOffset || minY > largeOffset || minZ > largeOffset ||
                           maxX < -largeOffset || maxY < -largeOffset || maxZ < -largeOffset;

    if (!isLarge && !isFarAway && !isSmall)
        return;

    QString msg = QString("Note: The loaded model's size is %1m x %2m x %3m\n\n").arg(width).arg(height).arg(depth);

    if (isLarge)
    {
        msg.append("* It is very large along at least one axis\n");
    }

    if (isSmall)
    {
        msg.append("* It is very small along at least one axis\n");
    }

    if (isFarAway)
    {
        msg.append("* It is far away from the origin\n");
    }

    msg.append("\nThese properties can make it difficult to see the model or navigate around it. Here are a few tips:\n\n");
    msg.append("* Use the 'Model Scale' option from the toolbar to adjust its size\n");
    msg.append("* Use the tree view on the left to select the entire model or parts of it\n");
    msg.append("* Press the 'F' key to focus the camera on the selected part\n");
    msg.append("* Adjust the camera movement speed.\n");

    QMessageBox::information(nullptr, "Model Information", msg, QMessageBox::Ok, QMessageBox::Ok);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Statistics
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ArrSession::UpdatePerformanceStatistics()
{
    if (m_arrSession == nullptr || m_arrSession->GetGraphicsBinding() == nullptr)
        return;

    RR::FrameStatistics stats;
    if (m_arrSession->GetGraphicsBinding()->GetLastFrameStatistics(&stats) != RR::Result::Success)
        return;

#define accumulate(value) m_frameStats.value = std::max(m_frameStats.value, stats.value);

    accumulate(LatencyPoseToReceive);
    accumulate(LatencyReceiveToPresent);
    accumulate(LatencyPresentToDisplay);
    accumulate(TimeSinceLastPresent);
    accumulate(VideoFrameReusedCount);
    accumulate(VideoFramesSkipped);
    accumulate(VideoFramesReceived);
    accumulate(VideoFramesDiscarded);
    accumulate(VideoFrameMinDelta);
    accumulate(VideoFrameMaxDelta);

#undef accumulate

    if (m_frameStatsUpdateDelay > 0)
    {
        --m_frameStatsUpdateDelay;
        return;
    }

    m_frameStatsUpdateDelay = 60;
    Q_EMIT FrameStatisticsChanged();

    m_frameStats = stats;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Session Lease Time
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ArrSession::SetAutoExtensionMinutes(int extendByMinutes)
{
    m_extensionMinutes = extendByMinutes;
}

void ArrSession::ChangeSessionLeaseTime(int totalLeaseMinutes)
{
    if (m_arrSession == nullptr || m_renewAsyncInProgress)
        return;

    RR::RenderingSessionUpdateOptions params;
    params.MaxLeaseInMinutes = totalLeaseMinutes;

    m_renewAsyncInProgress = true;

    // this callback might get called on any worker thread
    auto onRenew = [this](RR::Status status, RR::ApiHandle<RR::SessionContextResult> result)
    {
        // invoke the lambda on the main thread, so that it can access Qt elements
        QMetaObject::invokeMethod(QApplication::instance(), [this, status, result]
                                  {
                                      RR::Result errorCode = RR::StatusToResult(status);

                                      if (status == RR::Status::OK)
                                      {
                                          errorCode = result->GetErrorCode();
                                      }

                                      if (errorCode != RR::Result::Success)
                                      {
                                          qCritical(LoggingCategory::RenderingSession) << "Extending session time failed: " << errorCode;
                                      }

                                      m_renewAsyncInProgress = false;
                                      UpdateSessionProperties(); });
    };

    m_arrSession->RenewAsync(params, onRenew);
}

void ArrSession::ExtendSessionIfNeeded()
{
    if (m_extensionMinutes == 0)
        return;

    const auto status = GetSessionStatus();

    if (!status.IsRunning() || status.m_state == ArrSessionStatus::State::StartRequested)
        return;

    const int remainingMinutes = status.m_leaseTimeInMinutes - status.m_elapsedTimeInMinutes;

    // once the remaining time gets close to the auto-extension time, we update the session lease time
    // such that the desired extension time always remains
    if (remainingMinutes <= m_extensionMinutes)
    {
        // we always extend an extra minute, otherwise we'd be extending the session every update
        ChangeSessionLeaseTime(status.m_elapsedTimeInMinutes + m_extensionMinutes + 1);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// ARR Inspector
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


void ArrSession::StartArrInspector()
{
    if (m_arrSession == nullptr || m_connectToArrInspectorInProgress)
        return;

    m_connectToArrInspectorInProgress = true;

    auto onConnected = [this](RR::Status status, std::string result)
    {
        if (status != RR::Status::OK)
        {
            qWarning(LoggingCategory::RenderingSession)
                << "Failed to create connection to ArrInspector: " << status;

            QMessageBox::warning(nullptr, "Starting ArrInspector failed", QString("Failed to create connection to ArrInspector.\n\nReason: %1").arg(toString(status)), QMessageBox::Ok);
        }
        else
        {
            // try and start a browser
            if (!QDesktopServices::openUrl(QUrl::fromLocalFile(result.c_str())))
            {
                QMessageBox::warning(nullptr, "Starting ArrInspector failed", QString("Failed to open a browser to the URL:\n%1").arg(result.c_str()), QMessageBox::Ok);
            }
        }

        m_connectToArrInspectorInProgress = false;
    };

    m_arrSession->ConnectToArrInspectorAsync(onConnected);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Model Loading
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool ArrSession::LoadModel(const QString& modelName, const char* assetSAS)
{
    auto api = GetRenderingConnection();
    if (api == nullptr)
        return false;

    QPointer<ArrSession> thisPtr = this;
    int loadIdx = (int)m_loadingProgress.size();
    m_loadingProgress.push_back(0.01f);

    // the callback is called from the GUI thread
    auto onModelLoaded = [thisPtr, modelName, loadIdx](RR::Status status, RR::ApiHandle<RR::LoadModelResult> loadResult)
    {
        thisPtr->m_loadingProgress[loadIdx] = 1.0;

        if (status != RR::Status::OK)
        {
            qCritical(LoggingCategory::RenderingSession)
                << "Loading model " << modelName << " failed: " << status;
        }
        else
        {
            if (loadResult.valid())
            {
                auto root = loadResult->GetRoot();

                const float scale = thisPtr->m_modelScale;
                root->SetScale(RR::Float3{scale, scale, scale});

                thisPtr->m_loadedModels.push_back({});
                auto& res = thisPtr->m_loadedModels.back();
                res.m_ModelName = modelName;
                res.m_LoadResult = std::move(loadResult);

                Q_EMIT thisPtr->ModelLoaded();

                thisPtr->CheckEntityBounds(root);
            }
            else
            {
                qCritical(LoggingCategory::RenderingSession)
                    << "Loading model " << modelName << " failed.";
            }
        }

        Q_EMIT thisPtr->ModelLoadProgressChanged();
    };

    auto onModelLoadingProgress = [thisPtr, loadIdx](float progress)
    {
        thisPtr->m_loadingProgress[loadIdx] = std::max(thisPtr->m_loadingProgress[loadIdx], progress);

        Q_EMIT thisPtr->ModelLoadProgressChanged();
    };

    RR::LoadModelFromSasOptions params;
    params.ModelUri = assetSAS;
    api->LoadModelFromSasAsync(params, onModelLoaded, onModelLoadingProgress);

    return true;
}

void ArrSession::RemoveModel(size_t idx)
{
    m_selectedEntities.erase(m_loadedModels[idx].m_LoadResult->GetRoot()->GetHandle());

    m_loadedModels[idx].m_LoadResult->GetRoot()->Destroy();

    if (m_loadedModels.size() > 1)
    {
        m_loadedModels[idx] = m_loadedModels.back();
    }

    m_loadedModels.pop_back();

    Q_EMIT ModelLoaded();
}

float ArrSession::GetModelLoadingProgress() const
{
    float totalProgress = 1.0f;

    for (auto prog : m_loadingProgress)
    {
        if (prog < totalProgress)
        {
            totalProgress = prog;
        }
    }

    return totalProgress;
}

void ArrSession::SetModelScale(float scale)
{
    m_modelScale = scale;

    for (auto& model : m_loadedModels)
    {
        model.m_LoadResult->GetRoot()->SetScale(RR::Float3{m_modelScale, m_modelScale, m_modelScale});
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Object selection
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ArrSession::SetEntitySelected(const RR::ApiHandle<RR::Entity>& entity, bool enable)
{
    if (!entity->GetValid())
        return;

    if (enable)
        m_selectedEntities[entity->GetHandle()] = entity;
    else
        m_selectedEntities.erase(entity->GetHandle());

    std::vector<RR::ApiHandle<RR::ComponentBase>> components;
    entity->GetComponents(components);

    for (auto& comp : components)
    {
        if (comp->GetType() == RR::ObjectType::HierarchicalStateOverrideComponent)
        {
            if (enable)
            {
                // component already exists -> nothing to do
                return;
            }
            else
            {
                // remove component to deselect entity
                comp->Destroy();
                return;
            }
        }
    }

    if (!enable)
        return;

    // through the ARR RenderingConnection we can modify the scene
    if (auto component = m_arrSession->Connection()->CreateComponent(RR::ObjectType::HierarchicalStateOverrideComponent, entity))
    {
        if (auto hierarchicalComp = component->as<RR::HierarchicalStateOverrideComponent>())
        {
            hierarchicalComp->SetState(RR::HierarchicalStates::Selected, RR::HierarchicalEnableState::ForceOn);
        }
    }
}

void ArrSession::GetSelectedEntities(std::vector<RR::ApiHandle<RR::Entity>>& selected) const
{
    selected.clear();
    selected.reserve(m_selectedEntities.size());

    for (auto it : m_selectedEntities)
    {
        selected.push_back(it.second);
    }
}