#pragma once

#include <QObject>

#include <QElapsedTimer>
#include <Rendering/IncludeAzureRemoteRendering.h>
#include <deque>

namespace Microsoft::Azure::RemoteRendering
{
    class AzureSession;
    class RemoteRenderingClient;
    using LoadResult = std::function<void(Status, ApiHandle<Entity>)>;
    using LoadProgress = std::function<void(float)>;
} // namespace Microsoft::Azure::RemoteRendering


class ArrAccount;
class QTimer;
class SceneState;

struct ArrSessionStatus
{
    enum class State
    {
        NotActive,
        Stopped,
        Expired,
        Error,

        StartRequested,
        Starting,
        ReadyNotConnected,
        ReadyConnecting,
        ReadyConnected,
    };

    bool IsRunning() const;

    State m_state = State::NotActive;
    int m_elapsedTimeInMinutes = 0;
    int m_leaseTimeInMinutes = 0;
};

class ArrSession : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void SessionStatusChanged();
    void ModelLoaded();
    void ModelLoadProgressChanged();
    void FrameStatisticsChanged();

public:
    ArrSession(ArrAccount* arrClient, SceneState* sceneState);
    ~ArrSession();

    void CreateSession(const RR::RenderingSessionCreationOptions& info);
    void OpenSession(const QString& sessionID);
    void CloseSession(bool keepRunning);

    ArrSessionStatus GetSessionStatus() const;

    QString GetSessionUuid() const;

    void StartArrInspector();

    /// Updates the session lease time to the given duration.
    ///
    /// This may be shorter or longer than the original lease time.
    void ChangeSessionLeaseTime(int totalLeaseMinutes);

    void SetAutoExtensionMinutes(int extendByMinutes);

    struct LoadedModel
    {
        QString m_ModelName;
        RR::ApiHandle<RR::LoadModelResult> m_LoadResult;
    };

    bool LoadModel(const QString& modelName, const char* assetSAS);
    void RemoveModel(size_t idx);
    float GetModelLoadingProgress() const;
    const std::deque<LoadedModel>& GetLoadedModels() const { return m_loadedModels; }

    void EnableSelectionOutline(const RR::ApiHandle<RR::Entity>& entity, bool enable);
    void GetSelectedEntities(std::vector<RR::ApiHandle<RR::Entity>>& selected) const;

    const RR::FrameStatistics& GetFrameStatistics() const { return m_frameStats; }

private:
    void OpenOrCreateSessionResult(RR::Status status, RR::ApiHandle<RR::CreateRenderingSessionResult> result);
    void ConnectToSessionRuntime();
    void ConnectToSessionRuntimeResult(RR::Status status, RR::ConnectionStatus result);
    void DisconnectFromSessionRuntime();
    void GetSessionPropertiesResult(RR::Status status, RR::ApiHandle<RR::RenderingSessionPropertiesResult> result);
    void OnSessionPropertiesUpdated();
    void OnConnectionStatusChanged(RR::ConnectionStatus status, RR::Result result);
    void ExtendSessionIfNeeded();
    void OnSceneRefresh();
    void UpdateSessionProperties();
    void UpdatePerformanceStatistics();
    // start/stop internal timers based on the session state
    void ConfigureSessionPropertiesUpdateTimer();

    RR::ApiHandle<RR::RenderingConnection> GetRenderingConnection();

    ArrAccount* m_arrClient = nullptr;
    RR::ApiHandle<RR::RenderingSession> m_arrSession = nullptr;

    int m_extensionMinutes = 0;

    SceneState* m_sceneState = nullptr;

    QElapsedTimer m_connectingElapsedTime;
    QTimer* m_updateSessionPropertiesTimer = nullptr;

    std::atomic_bool m_createSessionInProgress = false;
    std::atomic_bool m_connectingInProgress = false;
    std::atomic_bool m_connectToArrInspectorInProgress = false;
    std::atomic_bool m_renewAsyncInProgress = false;

    // cached properties coming from RR::RenderingSession::GetRenderingSessionPropertiesAsync
    RR::RenderingSessionProperties m_lastProperties = {};

    RR::event_token m_statusChangedToken;
    RR::event_token m_messageLoggedToken;

    std::vector<float> m_loadingProgress;
    std::deque<LoadedModel> m_loadedModels;
    std::map<unsigned long long, RR::ApiHandle<RR::Entity>> m_selectedEntities;

    int m_frameStatsUpdateDelay = 60;
    RR::FrameStatistics m_frameStats;
};