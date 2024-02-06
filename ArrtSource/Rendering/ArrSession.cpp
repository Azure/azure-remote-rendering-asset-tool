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
#include <mutex>
#include <windows.h>

QString FormatTime(int msecs)
{
    const int hours = msecs / (60 * 60 * 1000);
    msecs -= hours * (60 * 60 * 1000);
    const int minutes = msecs / (60 * 1000);
    msecs -= minutes * (60 * 1000);
    const int seconds = msecs / 1000;
    msecs -= seconds * 1000;

    return QString(" [%1:%2:%3.%4]").arg(hours, 2, 10, (QLatin1Char)'0').arg(minutes, 2, 10, (QLatin1Char)'0').arg(seconds, 2, 10, (QLatin1Char)'0').arg(msecs, 3, 10, (QLatin1Char)'0');
}

void ArrSession::OnConnectionStateChanged()
{
    if (m_previousState != m_ConnectionLogic.GetCurrentState())
    {
        const int timeDiffMsecs = m_previousStateChange.msecsTo(QTime::currentTime());
        m_previousStateChange = QTime::currentTime();

        if (m_previousState == ArrConnectionLogic::State::RuntimeConnected && m_ConnectionLogic.GetCurrentState() == ArrConnectionLogic::State::SessionReady)
        {
            qWarning(LoggingCategory::RenderingSession) << "Rendering session got disconnected, most likely due to a network timeout. Loaded models will be cleared." << FormatTime(timeDiffMsecs).toUtf8().data();
        }

        qInfo(LoggingCategory::RenderingSession) << "New connection state: " << ArrConnectionLogic::GetStateString(m_ConnectionLogic.GetCurrentState()) << FormatTime(timeDiffMsecs).toUtf8().data();

        m_previousState = m_ConnectionLogic.GetCurrentState();

        if (!m_ConnectionLogic.IsConnectionRendering())
        {
            std::lock_guard<std::recursive_mutex> lk(m_modelMutex);

            m_loadingProgress.clear();
            m_loadedModels.clear();
            m_selectedEntities.clear();
        }
    }

    Q_EMIT SessionStatusChanged();
}

void ArrSession::OnSessionPropertiesUpdated()
{
    ExtendSessionIfNeeded();
}

void ArrSession::OnInitGrahpcs()
{
    m_sceneState->SetSession(m_ConnectionLogic.GetArrSession(), this);
}

void ArrSession::OnDeinitGrahpcs()
{
    m_sceneState->SetSession(nullptr, this);

    {
        std::lock_guard<std::recursive_mutex> lk(m_modelMutex);
        m_loadingProgress.clear();
        m_loadedModels.clear();
        m_selectedEntities.clear();
    }

    Q_EMIT ModelLoadProgressChanged();
}

ArrSession::ArrSession(ArrAccount* arrAccount, SceneState* sceneState)
{
    m_arrAccount = arrAccount;
    m_sceneState = sceneState;

    connect(sceneState, &SceneState::SceneRefreshed, this, [this]()
            { OnSceneRefresh(); });

    connect(&m_ConnectionLogic, &ArrConnectionLogic::ConnectionStateChanged, this, &ArrSession::OnConnectionStateChanged);
    connect(&m_ConnectionLogic, &ArrConnectionLogic::InitGraphics, this, &ArrSession::OnInitGrahpcs, Qt::DirectConnection);
    connect(&m_ConnectionLogic, &ArrConnectionLogic::DeinitGraphics, this, &ArrSession::OnDeinitGrahpcs, Qt::DirectConnection);
    connect(&m_ConnectionLogic, &ArrConnectionLogic::SessionPropertiesUpdated, this, &ArrSession::OnSessionPropertiesUpdated);

    m_frameStatsTimer.start();
}

ArrSession::~ArrSession()
{
    if (m_ConnectionLogic.IsConnectionStoppable())
    {
        m_ConnectionLogic.CloseSession(false);
    }
}

RR::ApiHandle<RR::RenderingConnection> ArrSession::GetRenderingConnection()
{
    return m_ConnectionLogic.GetArrRenderingConnection();
}

QString ArrSession::GetSessionID() const
{
    if (!m_ConnectionLogic.GetArrSession())
    {
        return "no active session";
    }

    std::string sessionUuid;
    m_ConnectionLogic.GetArrSession()->GetSessionUuid(sessionUuid);
    return sessionUuid.c_str();
}

void ArrSession::CreateSession(const RR::RenderingSessionCreationOptions& info)
{
    if (m_arrAccount->GetClient() == nullptr)
    {
        m_ConnectionLogic.MockNewSession();
        return;
    }
    m_ConnectionLogic.CreateNewSession(m_arrAccount->GetClient(), info);
}

void ArrSession::OpenSession(const QString& sessionID)
{
    if (m_arrAccount->GetClient() == nullptr)
    {
        m_ConnectionLogic.MockNewSession();
        return;
    }
    m_ConnectionLogic.OpenExistingSession(m_arrAccount->GetClient(), sessionID);
}

void ArrSession::CloseSession(bool keepRunning)
{
    m_ConnectionLogic.CloseSession(keepRunning);
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

void ArrSession::AggregateStats()
{
    m_numFrames = m_frameStatsHistory.size();

    if (m_numFrames == 0)
    {
        return;
    }

    // clean up saved stats
    m_frameStats = RR::FrameStatistics{};

    for (auto& frameStats : m_frameStatsHistory)
    {
        m_frameStats.LatencyPoseToReceive += frameStats.LatencyPoseToReceive;
        m_frameStats.LatencyReceiveToPresent += frameStats.LatencyReceiveToPresent;
        m_frameStats.LatencyPresentToDisplay += frameStats.LatencyPresentToDisplay;
        m_frameStats.TimeSinceLastPresent += frameStats.TimeSinceLastPresent;

        m_frameStats.VideoFrameReusedCount += frameStats.VideoFrameReusedCount > 0 ? 1 : 0;
        m_frameStats.VideoFramesSkipped += frameStats.VideoFramesSkipped;
        m_frameStats.VideoFramesReceived += frameStats.VideoFramesReceived;
        m_frameStats.VideoFramesDiscarded += frameStats.VideoFramesDiscarded;

        if (m_frameStats.VideoFramesReceived > 0)
        {
            if (m_frameStats.VideoFrameMinDelta == 0.0f)
            {
                m_frameStats.VideoFrameMinDelta = m_frameStats.VideoFrameMinDelta;
                m_frameStats.VideoFrameMaxDelta = m_frameStats.VideoFrameMaxDelta;
            }
            else
            {
                m_frameStats.VideoFrameMinDelta = std::min(m_frameStats.VideoFrameMinDelta, frameStats.VideoFrameMinDelta);
                m_frameStats.VideoFrameMaxDelta = std::max(m_frameStats.VideoFrameMaxDelta, frameStats.VideoFrameMaxDelta);
            }
        }
    }

    const float oneOverNumFrames = 1.0f / m_numFrames;
    m_frameStats.LatencyPoseToReceive *= oneOverNumFrames;
    m_frameStats.LatencyReceiveToPresent *= oneOverNumFrames;
    m_frameStats.LatencyPresentToDisplay *= oneOverNumFrames;
    m_frameStats.TimeSinceLastPresent *= oneOverNumFrames;

    // clean up the history
    m_frameStatsHistory.clear();
}

void ArrSession::UpdatePerformanceStatistics()
{
    if (!m_ConnectionLogic.GetArrSession() || m_ConnectionLogic.GetArrSession()->GetGraphicsBinding() == nullptr)
        return;

    RR::FrameStatistics stats;
    if (m_ConnectionLogic.GetArrSession()->GetGraphicsBinding()->GetLastFrameStatistics(&stats) != RR::Result::Success)
        return;

    m_frameStatsHistory.push_back(stats);

    if (m_frameStatsTimer.elapsed() < 1000)
    {
        return;
    }

    m_frameStatsTimer.restart();
    AggregateStats();
    Q_EMIT FrameStatisticsChanged();
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
    if (!m_ConnectionLogic.GetArrSession() || m_renewAsyncInProgress.exchange(true))
        return;

    RR::RenderingSessionUpdateOptions params;
    params.MaxLeaseInMinutes = totalLeaseMinutes;

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
            
                                      
                                      /* TODO UpdateSessionProperties();*/ });
    };

    m_ConnectionLogic.GetArrSession()->RenewAsync(params, onRenew);
}

void ArrSession::ExtendSessionIfNeeded()
{
    if (m_extensionMinutes == 0)
        return;

    if (!m_ConnectionLogic.IsConnectionStoppable())
        return;

    const int remainingMinutes = m_ConnectionLogic.GetLeaseTimeInMinutes() - m_ConnectionLogic.GetElapsedTimeInMinutes();

    // once the remaining time gets close to the auto-extension time, we update the session lease time
    // such that the desired extension time always remains
    if (remainingMinutes <= m_extensionMinutes)
    {
        // we always extend an extra minute, otherwise we'd be extending the session every update
        ChangeSessionLeaseTime(m_ConnectionLogic.GetElapsedTimeInMinutes() + m_extensionMinutes + 1);
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
    if (!m_ConnectionLogic.GetArrSession() || m_connectToArrInspectorInProgress.exchange(true))
        return;

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

    m_ConnectionLogic.GetArrSession()->ConnectToArrInspectorAsync(onConnected);
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

    int loadIdx = 0;
    {
        std::lock_guard<std::recursive_mutex> lk(thisPtr->m_modelMutex);
        loadIdx = (int)m_loadingProgress.size();
        m_loadingProgress.push_back(0.01f);
    }

    qInfo(LoggingCategory::RenderingSession) << "Loading model " << modelName;

    const QTime startTime = QTime::currentTime();

    // the callback is called from the GUI thread
    auto onModelLoaded = [thisPtr, modelName, loadIdx, startTime](RR::Status status, RR::ApiHandle<RR::LoadModelResult> loadResult)
    {
        std::lock_guard<std::recursive_mutex> lk(thisPtr->m_modelMutex);

        if (loadIdx < thisPtr->m_loadingProgress.size())
        {
            thisPtr->m_loadingProgress[loadIdx] = 1.0;
        }

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

                {
                    std::lock_guard<std::recursive_mutex> lk2(thisPtr->m_modelMutex);
                    thisPtr->m_loadedModels.push_back({});
                    auto& res = thisPtr->m_loadedModels.back();
                    res.m_ModelName = modelName;
                    res.m_LoadResult = std::move(loadResult);
                }

                Q_EMIT thisPtr->ModelLoaded();

                const int timeDiffMsecs = startTime.msecsTo(QTime::currentTime());

                qInfo(LoggingCategory::RenderingSession) << "Finished loading model " << modelName << FormatTime(timeDiffMsecs).toUtf8().data();

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
        std::lock_guard<std::recursive_mutex> lk(thisPtr->m_modelMutex);

        if (loadIdx < thisPtr->m_loadingProgress.size())
        {
            thisPtr->m_loadingProgress[loadIdx] = std::max(thisPtr->m_loadingProgress[loadIdx], progress);
        }

        Q_EMIT thisPtr->ModelLoadProgressChanged();
    };

    RR::LoadModelFromSasOptions params;
    params.ModelUri = assetSAS;

    api->LoadModelFromSasAsync(params, onModelLoaded, onModelLoadingProgress);

    return true;
}

void ArrSession::RemoveModel(size_t idx)
{
    {
        std::lock_guard<std::recursive_mutex> lk(m_modelMutex);

        m_selectedEntities.erase(m_loadedModels[idx].m_LoadResult->GetRoot()->GetHandle());

        m_loadedModels[idx].m_LoadResult->GetRoot()->Destroy();

        if (m_loadedModels.size() > 1)
        {
            m_loadedModels[idx] = m_loadedModels.back();
        }

        m_loadedModels.pop_back();
    }

    Q_EMIT ModelLoaded();
}

float ArrSession::GetModelLoadingProgress() const
{
    float totalProgress = 2.0f;

    std::lock_guard<std::recursive_mutex> lk(m_modelMutex);

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

    std::lock_guard<std::recursive_mutex> lk(m_modelMutex);

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
    if (auto component = m_ConnectionLogic.GetArrRenderingConnection()->CreateComponent(RR::ObjectType::HierarchicalStateOverrideComponent, entity))
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